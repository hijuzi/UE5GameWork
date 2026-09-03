// Fill out your copyright notice in the Description page of Project Settings.

#include "Combat/Turn/SVCombatTurnCoordinator.h"

#include "Combat/SVCombatCoreInterface.h"
#include "Combat/SVCombatDataStore.h"
#include "Combat/SVCombatManagerSubsystem.h"
#include "Combat/SVCombatRoundTimer.h"
#include "Combat/Component/SVCharacterTurnComponent.h"
#include "GameFramework/Character.h"
#include "CatCombatLog.h"
#include "TimerManager.h"
#include "UObject/Class.h"

void USVCombatTurnCoordinator::Initialize(USVCombatManagerSubsystem* InOwner, USVCombatDataStore* InDataStore)
{
	OwnerSubsystem = InOwner;
	DataStore = InDataStore;
}

void USVCombatTurnCoordinator::Shutdown()
{
	PendingReadySet.Empty();
	CampStartCount.Reset();
	Phase = ECombatCampPhase::Idle;
	PreviousPhase = ECombatCampPhase::Idle;
	CurrentTeam = ECombatTeamType::Player;
	OwnerSubsystem = nullptr;
	DataStore = nullptr;
}

void USVCombatTurnCoordinator::SetPhase(ECombatCampPhase NewPhase)
{
	if (Phase == NewPhase)
	{
		return;
	}
	// 记录上一个阶段（调试/观测用）
	PreviousPhase = Phase;
	Phase = NewPhase;
	OnTurnPhaseChanged.ExecuteIfBound(Phase, CurrentTeam);
	UE_LOG(LogCatCombatCoordinator, Verbose, TEXT("TurnCoordinator: Phase %s -> %s, Team=%s"),
		*UEnum::GetValueAsString(PreviousPhase), *UEnum::GetValueAsString(Phase), *UEnum::GetValueAsString(CurrentTeam));
}

ECombatTeamType USVCombatTurnCoordinator::GetOpponentTeam(ECombatTeamType TeamType) const
{
	return TeamType == ECombatTeamType::Player ? ECombatTeamType::Enemy : ECombatTeamType::Player;
}

bool USVCombatTurnCoordinator::AreAllTeamsActed() const
{
	// 缓存枚举反射（静态，仅初始化一次）
	static const UEnum* TeamEnum = StaticEnum<ECombatTeamType>();
	if (!TeamEnum)
	{
		return false;
	}
	// 排除末尾 UHT 自动生成的 MAX 标记值
	const int32 EnumCount = TeamEnum->NumEnums() - 1;
	for (int32 i = 0; i < EnumCount; ++i)
	{
		const ECombatTeamType Team = static_cast<ECombatTeamType>(TeamEnum->GetValueByIndex(i));
		const int32* Count = CampStartCount.Find(Team);
		if (!Count || *Count < 1)
		{
			return false;
		}
	}
	return true;
}

bool USVCombatTurnCoordinator::IsAnyCharacterActing() const
{
	USVCombatDataStore* Store = DataStore.Get();
	if (!Store)
	{
		return false;
	}

	// 遍历所有战斗角色，检查角色级状态机是否处于 Acting（行动中）
	const TArray<ACharacter*> AllCharacters = Store->GetAllCharacters();
	for (ACharacter* Character : AllCharacters)
	{
		if (!Character)
		{
			continue;
		}
		const USVCharacterTurnComponent* TurnComp = Character->FindComponentByClass<USVCharacterTurnComponent>();
		if (TurnComp && TurnComp->GetState() == ECharacterTurnState::Acting)
		{
			// Acting 状态但当前激活 Tag 为空：状态与激活缓存不一致，输出警告
			if (!TurnComp->GetCurrentActingTag().IsValid())
			{
				UE_LOG(LogCatCombatCoordinator, Warning, TEXT("TurnCoordinator::IsAnyCharacterActing: %s 处于 Acting 但当前激活 Tag 为空"),
					*GetNameSafe(Character));
			}
			return true;
		}
	}

	return false;
}

void USVCombatTurnCoordinator::StartCamp(ECombatTeamType TeamType)
{
	USVCombatDataStore* Store = DataStore.Get();
	if (!Store)
	{
		UE_LOG(LogCatCombatCoordinator, Warning, TEXT("TurnCoordinator::StartCamp: DataStore 无效"));
		return;
	}

	// 无论是切队伍还是切回合，定时器都会从头开始
	if (USVCombatManagerSubsystem* Owner = OwnerSubsystem.Get())
	{
		if (USVCombatRoundTimer* RoundTimer = Owner->GetRoundTimer())
		{
			RoundTimer->ResetRoundTime();
		}
	}

	// 清空上一回合遗留的集合（防御性兜底）
	PendingReadySet.Empty();

	// 阵营 StartCamp 计数 +1（用于区分「切换队伍」与「切换下一回合」）
	int32& Count = CampStartCount.FindOrAdd(TeamType);
	++Count;

	CurrentTeam = TeamType;
	Store->SetCurrentTurnTeam(TeamType);

	SetPhase(ECombatCampPhase::Starting);

	// 阵营快照：攻击方角色（Attacker）+ 防守方角色（Defender）
	// 攻击方加入 PendingReadySet 并定向通知；防守方仅就位（不加入集合）
	const TArray<ACharacter*> Attackers = Store->GetCharacters(TeamType);
	for (ACharacter* Character : Attackers)
	{
		PendingReadySet.Add(Character);
		if (Character->Implements<USVCombatCoreInterface>())
		{
			ISVCombatCoreInterface::Execute_OnBeginTurn(Character, ECombatTurnRole::Attacker);
		}
	}

	const TArray<ACharacter*> Defenders = Store->GetCharacters(GetOpponentTeam(TeamType));
	for (ACharacter* Character : Defenders)
	{
		if (Character->Implements<USVCombatCoreInterface>())
		{
			ISVCombatCoreInterface::Execute_OnBeginTurn(Character, ECombatTurnRole::Defender);
		}
	}

	SetPhase(ECombatCampPhase::Acting);

	// 攻击方为空（例如开局即全灭）：先判断战斗是否结束，结束则直接进入结算
	if (PendingReadySet.Num() == 0)
	{
		if (Store->CheckCombatEnded())
		{
			// 进入结算阶段，并通知持有者执行结算
			SetPhase(ECombatCampPhase::Settle);
			if (USVCombatManagerSubsystem* Owner = OwnerSubsystem.Get())
			{
				Owner->SettleCombat();
			}
		}
		else
		{
			// 未结束（如敌方仅剩防守角色等情况），正常阵营收尾切换
			EndCamp();
		}
	}
}

void USVCombatTurnCoordinator::NotifyActionStarted(ACharacter* Character)
{
	// 单次行动开始：当前不改变集合（角色组件自驱动维护个体阶段）
	UE_LOG(LogCatCombatCoordinator, Verbose, TEXT("TurnCoordinator::NotifyActionStarted: %s"),
		Character ? *Character->GetName() : TEXT("NULL"));
}

void USVCombatTurnCoordinator::NotifyActionFinished(ACharacter* Character)
{
	// 单次行动结束：当前不改变集合（角色组件决定继续下一个待办还是 NotifyTurnFinished）
	UE_LOG(LogCatCombatCoordinator, Verbose, TEXT("TurnCoordinator::NotifyActionFinished: %s"),
		Character ? *Character->GetName() : TEXT("NULL"));
}

void USVCombatTurnCoordinator::NotifyTurnFinished(ACharacter* Character)
{
	if (!Character)
	{
		return;
	}

	const int32 Removed = PendingReadySet.Remove(Character);
	UE_LOG(LogCatCombatCoordinator, Verbose, TEXT("TurnCoordinator::NotifyTurnFinished: %s, Removed=%d, Remain=%d"),
		*Character->GetName(), Removed, PendingReadySet.Num());

	// 集合清空 → 阵营收尾
	if (PendingReadySet.Num() == 0)
	{
		EndCamp();
	}
}

void USVCombatTurnCoordinator::AbortCurrentCamp()
{
	PendingReadySet.Empty();
	SetPhase(ECombatCampPhase::Idle);
	UE_LOG(LogCatCombatCoordinator, Log, TEXT("TurnCoordinator::AbortCurrentCamp: 已中止当前阵营回合"));
}

void USVCombatTurnCoordinator::ResetCampStartCount()
{
	CampStartCount.Reset();
}

void USVCombatTurnCoordinator::EnterFinishing()
{
	// 进入收尾阶段前，先清理阵营状态
	CleanupCampState();

	// 已在 Finishing 阶段则无需重复切换，避免重复广播
	if (Phase != ECombatCampPhase::Finishing)
	{
		SetPhase(ECombatCampPhase::Finishing);
	}
}

bool USVCombatTurnCoordinator::TryEnterFinishing()
{
	// 只要仍有角色 Acting，就进入/保持「行动收尾（Finishing）」阶段（每次调用都重新检查）
	if (IsAnyCharacterActing())
	{
		EnterFinishing();
		UE_LOG(LogCatCombatCoordinator, Log, TEXT("TurnCoordinator::TryEnterFinishing: 仍有角色 Acting，进入行动收尾阶段"));
		return true;
	}
	return false;
}

void USVCombatTurnCoordinator::CleanupCampState()
{
	// 阵营收尾清理：暂不处理回合临时 Buff（后续第 6/7 步接入）
	PendingReadySet.Empty();
}

void USVCombatTurnCoordinator::EndCamp()
{
	// 仍有角色 Acting 时，先进入「行动收尾（Finishing）」阶段，暂不推进回合
	if (TryEnterFinishing())
	{
		return;
	}

	// 阵营收尾清理
	CleanupCampState();

	SetPhase(ECombatCampPhase::CampEnd);

	// 调度阵营推进（切换队伍 / 切换下一回合）
	ScheduleAdvanceCombatTurn();
}

void USVCombatTurnCoordinator::ScheduleAdvanceCombatTurn()
{
	// 阵营推进交由持有者统一编排。
	// 延迟到下一帧执行，切断 StartCamp → EndCamp → AdvanceCombatTurn → StartCamp 的同步递归链（空阵营场景防栈溢出）。
	USVCombatManagerSubsystem* Owner = OwnerSubsystem.Get();
	if (!Owner)
	{
		return;
	}
	UWorld* World = Owner->GetWorld();
	if (!World)
	{
		UE_LOG(LogCatCombatCoordinator, Warning, TEXT("TurnCoordinator::ScheduleAdvanceCombatTurn: World 无效，无法延迟推进战斗回合"));
		return;
	}
	World->GetTimerManager().SetTimerForNextTick([Owner]()
	{
		Owner->AdvanceCombatTurn();
	});
}
