// Fill out your copyright notice in the Description page of Project Settings.

#include "Combat/SVCombatManagerSubsystem.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "CatCombatGameplayTags.h"
#include "Combat/Actor/SVCombatScenePoint.h"
#include "Combat/SVCombatDataStore.h"
#include "Combat/SVCombatFunctionLibrary.h"
#include "Combat/SVCombatHUDHandler.h"
#include "Combat/SVCombatRoundTimer.h"
#include "Combat/Turn/SVCombatTurnCoordinator.h"
#include "GameFramework/Character.h"
#include "CatCombatLog.h"

bool USVCombatManagerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

void USVCombatManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	DataStore = NewObject<USVCombatDataStore>(this, USVCombatDataStore::StaticClass());
	DataStore->Initialize(this);

	HUDHandler = NewObject<USVCombatHUDHandler>(this, USVCombatHUDHandler::StaticClass());
	HUDHandler->Initialize(this);

	RoundTimer = NewObject<USVCombatRoundTimer>(this, USVCombatRoundTimer::StaticClass());
	RoundTimer->Initialize(this);
	RoundTimer->OnRoundTimeExpired.AddUObject(this, &USVCombatManagerSubsystem::HandleRoundTimeExpired);

	TurnCoordinator = NewObject<USVCombatTurnCoordinator>(this, USVCombatTurnCoordinator::StaticClass());
	TurnCoordinator->Initialize(this, DataStore);

	// 监听战斗角色加入/离开事件，联动输入映射
	OnCombatCharacterChanged.AddDynamic(this, &USVCombatManagerSubsystem::HandleCombatCharacterChanged);

	UE_LOG(LogCatCombatManager, Log, TEXT("CombatManagerSubsystem Initialized"));
}

void USVCombatManagerSubsystem::Deinitialize()
{
	OnCombatCharacterChanged.RemoveDynamic(this, &USVCombatManagerSubsystem::HandleCombatCharacterChanged);

	if (TurnCoordinator)
	{
		TurnCoordinator->Shutdown();
		TurnCoordinator = nullptr;
	}

	if (RoundTimer)
	{
		RoundTimer->Shutdown();
		RoundTimer = nullptr;
	}

	if (HUDHandler)
	{
		HUDHandler->Shutdown();
		HUDHandler = nullptr;
	}

	if (DataStore)
	{
		DataStore->Shutdown();
		DataStore = nullptr;
	}

	UE_LOG(LogCatCombatManager, Log, TEXT("CombatManagerSubsystem Deinitialized"));
	Super::Deinitialize();
}

void USVCombatManagerSubsystem::StartBattle(FName CombatConfigRowName)
{
	if (DataStore)
	{
		DataStore->CreateCombatData(CombatConfigRowName);
	}

	if (HUDHandler)
	{
		HUDHandler->CreateHUD();
	}

	if (RoundTimer)
	{
		RoundTimer->StartRoundTiming();
	}
	// 操控主玩家角色（执行层，逻辑移至 DataStore）
	if (DataStore)
	{
		DataStore->PossessMainPlayerCombatCharacter();
	}

	// 启动回合状态机：以配置的回合开始队伍（先手阵营）作为首个攻击方
	if (TurnCoordinator && DataStore)
	{
		TurnCoordinator->StartCamp(DataStore->GetStartingTeam());
	}
}

bool USVCombatManagerSubsystem::EndBattle(bool bClearTeam)
{
	if (DataStore)
	{
		const bool bEndCombat = DataStore->CheckCombatEnded();
		if (bEndCombat)
		{
			if (RoundTimer)
			{
				RoundTimer->StopRoundTiming();
			}

			if (HUDHandler)
			{
				HUDHandler->DestroyHUD();
			}
			// 清理战斗数据（数据层）
			DataStore->ClearCombatData(bClearTeam);
			return bEndCombat;
		}
	}
	return false;
}

void USVCombatManagerSubsystem::AdvanceToNextRound()
{
	if (DataStore)
	{
		DataStore->AdvanceToNextRound();
	}
	// 重置回回合开始队伍（先手阵营）作为首个攻击方
	if (TurnCoordinator && DataStore)
	{
		TurnCoordinator->StartCamp(DataStore->GetStartingTeam());
	}
}

void USVCombatManagerSubsystem::AdvanceCombatTurn()
{
	if (!TurnCoordinator || !DataStore)
	{
		UE_LOG(LogCatCombatManager, Warning, TEXT("AdvanceCombatTurn: 协调器或数据层未初始化"));
		return;
	}

	// 定时器重置统一由 TurnCoordinator::StartCamp 处理（切队伍/切回合都会调用）

	// 仍有角色 Acting 时，进入「行动收尾（Finishing）」阶段，暂不推进回合
	if (TurnCoordinator->TryEnterFinishing())
	{
		UE_LOG(LogCatCombatManager, Log, TEXT("AdvanceCombatTurn: 仍有角色 Acting，暂不推进回合"));
		return;
	}

	// 判断：所有阵营都行动过 → 切换下一回合；否则 → 切换队伍
	if (TurnCoordinator->AreAllTeamsActed())
	{
		// 切换下一回合：重置计数 + 推进到下一回合
		UE_LOG(LogCatCombatManager, Log, TEXT("AdvanceCombatTurn: 所有阵营均已行动，切换下一回合"));
		TurnCoordinator->ResetCampStartCount();
		AdvanceToNextRound();
	}
	else
	{
		// 切换队伍：切到对方阵营
		const ECombatTeamType NextTeam = TurnCoordinator->GetOpponentTeam(TurnCoordinator->GetCurrentTeam());
		UE_LOG(LogCatCombatManager, Log, TEXT("AdvanceCombatTurn: 切换队伍到 %s"), *UEnum::GetValueAsString(NextTeam));
		DataStore->SetCurrentTurnTeam(NextTeam);
		TurnCoordinator->StartCamp(NextTeam);
	}
}

ECombatResultType USVCombatManagerSubsystem::GetCombatResult() const
{
	return DataStore ? DataStore->GetResult() : ECombatResultType::Unsettled;
}

void USVCombatManagerSubsystem::SettleCombat()
{
	// 先取结算结果，再清理战斗（避免 EndBattle 清理数据影响结果读取）
	const ECombatResultType Result = GetCombatResult();
	UE_LOG(LogCatCombatManager, Log, TEXT("SettleCombat: Result=%s"), *UEnum::GetValueAsString(Result));

	EndBattle(true);

	// 广播结算事件（扩展点：外部剧情/流程系统绑定接入结算后续逻辑）
	OnCombatSettled.Broadcast(Result);
}

TArray<ACharacter*> USVCombatManagerSubsystem::GetCombatCharacterList(ECombatTeamType TeamType) const
{
	if (DataStore)
	{
		return DataStore->GetCharacters(TeamType);
	}
	return TArray<ACharacter*>();
}

void USVCombatManagerSubsystem::TryExecuteCombatCharacterAbility(bool bJoined, ACharacter* Character)
{
	if (!Character)
	{
		return;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Character);
	if (!ASC)
	{
		UE_LOG(LogCatCombatManager, Warning, TEXT("TryExecuteCombatCharacterAbility: 角色 %s 未配置 AbilitySystemComponent，跳过"),
			*GetNameSafe(Character));
		return;
	}

	const FGameplayTag AbilityTag = bJoined
		? CatCombatGameplayTags::TAG_ABILITY_COMBAT_JOIN
		: CatCombatGameplayTags::TAG_ABILITY_COMBAT_LEAVE;

	const bool bActivated = ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(AbilityTag));
	UE_LOG(LogCatCombatManager, Verbose, TEXT("TryExecuteCombatCharacterAbility: 角色 %s %s战斗，标签 %s，激活结果=%s"),
		*GetNameSafe(Character), bJoined ? TEXT("加入") : TEXT("离开"), *AbilityTag.ToString(),
		bActivated ? TEXT("成功") : TEXT("失败"));
}

float USVCombatManagerSubsystem::GetRoundProgress() const
{
	return RoundTimer ? RoundTimer->GetProgress() : 0.0f;
}

void USVCombatManagerSubsystem::HandleCombatCharacterChanged(bool bJoined, ECombatTeamType TeamType, ACharacter* Character)
{
	// 尝试执行角色加入/离开战斗的 Ability
	TryExecuteCombatCharacterAbility(bJoined, Character);
}

void USVCombatManagerSubsystem::HandleRoundTimeExpired()
{
	// 超时推进战斗回合：强制推进到下一阵营/下一回合（AdvanceCombatTurn 内部有判空保护）
	UE_LOG(LogCatCombatManager, Log, TEXT("HandleRoundTimeExpired: 回合计时超时，强制推进战斗回合"));
	AdvanceCombatTurn();
}

void USVCombatManagerSubsystem::RegisterScenePoint(ASVCombatScenePoint* ScenePoint)
{
	if (!ScenePoint)
	{
		return;
	}

	const FName RowName = ScenePoint->GetCombatConfigRowName();
	if (RowName.IsNone())
	{
		UE_LOG(LogCatCombatManager, Warning, TEXT("RegisterScenePoint: 场景点 %s 未配置 CombatConfigRowName，跳过注册"), *ScenePoint->GetName());
		return;
	}

	if (const TWeakObjectPtr<ASVCombatScenePoint>* Existing = ScenePointMap.Find(RowName))
	{
		// 仅当已存在有效注册且不是同一对象时视为重复，输出警告
		if (Existing->IsValid() && Existing->Get() != ScenePoint)
		{
			UE_LOG(LogCatCombatManager, Warning,
				TEXT("RegisterScenePoint: 配置行名 %s 已被场景点 %s 注册，场景点 %s 将覆盖前者（请检查关卡中是否有重复场景点）"),
				*RowName.ToString(), *Existing->Get()->GetName(), *ScenePoint->GetName());
		}
	}

	ScenePointMap.Add(RowName, ScenePoint);
	UE_LOG(LogCatCombatManager, Log, TEXT("RegisterScenePoint: 已注册场景点 %s（行名 %s）"), *ScenePoint->GetName(), *RowName.ToString());
}

void USVCombatManagerSubsystem::UnregisterScenePoint(ASVCombatScenePoint* ScenePoint)
{
	if (!ScenePoint)
	{
		return;
	}

	// 行名运行期不变，直接按 Key 移除；附加指针校验避免误删
	const FName RowName = ScenePoint->GetCombatConfigRowName();
	if (RowName.IsNone())
	{
		return;
	}

	if (const TWeakObjectPtr<ASVCombatScenePoint>* Existing = ScenePointMap.Find(RowName))
	{
		if (Existing->Get() == ScenePoint)
		{
			ScenePointMap.Remove(RowName);
			UE_LOG(LogCatCombatManager, Log, TEXT("UnregisterScenePoint: 已注销场景点 %s（行名 %s）"), *ScenePoint->GetName(), *RowName.ToString());
		}
	}
}

ASVCombatScenePoint* USVCombatManagerSubsystem::GetScenePoint(FName RowName) const
{
	if (const TWeakObjectPtr<ASVCombatScenePoint>* Found = ScenePointMap.Find(RowName))
	{
		return Found->Get();
	}
	return nullptr;
}
