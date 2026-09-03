// Fill out your copyright notice in the Description page of Project Settings.

#include "Combat/SVCombatDataStore.h"

#include "Combat/SVCombatCoreInterface.h"
#include "Combat/SVCombatFunctionLibrary.h"
#include "Combat/SVCombatManagerSubsystem.h"
#include "Combat/SVCombatAbilityGranter.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayEffectTypes.h"
#include "Combat/DataAsset/SVCombatCharacterDataAsset.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "CatCombatLog.h"
#include "Combat/SVCombatDataTable.h"
#include "Combat/Actor/SVCombatScenePoint.h"
#include "Camera/CameraActor.h"
#include "Engine/World.h"
#include "UObject/UnrealType.h"

void USVCombatDataStore::Initialize(USVCombatManagerSubsystem* InOwner)
{
	OwnerSubsystem = InOwner;
}

void USVCombatDataStore::Shutdown()
{
	Reset();
	OwnerSubsystem = nullptr;
}

void USVCombatDataStore::Reset()
{
	CharacterMap.Empty();
	CombatResult = ECombatResultType::Unsettled;
	bInCombat = false;
	CurrentTurnTeam = ECombatTeamType::Player;
	StartingTeam = ECombatTeamType::Player;
	ResetRoundNumber();
	CurrentScenePoint.Reset();
	CombatConfigRowName = NAME_None;
}

void USVCombatDataStore::CreateCombatData(const FName InCombatConfigRowName)
{
	ClearCombatData(true);
	bInCombat = true;
	// 战斗开始即第 1 回合（完整回合语义，不再从 0 起算）
	SetRoundNumber(1);
	// 缓存配置行名（调试/观测用）
	CombatConfigRowName = InCombatConfigRowName;

	USVCombatManagerSubsystem* Owner = OwnerSubsystem.Get();
	if (!Owner)
	{
		return;
	}

	// 场景点用于计算角色生成站位
	ASVCombatScenePoint* ScenePoint = Owner->GetScenePoint(InCombatConfigRowName);
	if (!ScenePoint)
	{
		return;
	}
	// 弱引用保存场景点，不管理其生命周期与 GC；场景点销毁后 GetScenePoint 自动返回 nullptr
	CurrentScenePoint = ScenePoint;

	const FSVCombatDataTableRow* Row = ScenePoint->GetCombatConfigRow();
	if (!Row)
	{
		return;
	}

	// 读取配置的回合开始队伍（先手阵营），并作为当前回合队伍
	StartingTeam = Row->StartingTeam;
	CurrentTurnTeam = StartingTeam;

	// 根据配置行中的阵容，在场景点站位处生成双方角色并注册进战斗
	UWorld* World = ScenePoint->GetWorld();
	if (!World)
	{
		UE_LOG(LogCatCombatDataStore, Warning, TEXT("CreateCombatData: 场景点 %s 所在世界无效"), *ScenePoint->GetName());
		return;
	}

	const int32 PlayerSpawned = SpawnTeamCharacters(World, ScenePoint, ECombatTeamType::Player, Row->PlayerTeams);
	const int32 EnemySpawned = SpawnTeamCharacters(World, ScenePoint, ECombatTeamType::Enemy, Row->EnemyTeams);
	UE_LOG(LogCatCombatDataStore, Log, TEXT("CreateCombatData: 配置行 [%s] 战斗角色已生成（玩家 %d 人 / 敌人 %d 人）"),
		*InCombatConfigRowName.ToString(), PlayerSpawned, EnemySpawned);
}

int32 USVCombatDataStore::SpawnTeamCharacters(UWorld* World, ASVCombatScenePoint* ScenePoint, ECombatTeamType TeamType, const TArray<TObjectPtr<USVCombatCharacterDataAsset>>& Teams)
{
	if (!World || !ScenePoint)
	{
		return 0;
	}

	int32 SpawnedCount = 0;
	// 队伍类别名取枚举短名（如 "Player"/"Enemy"）
	const FString TeamTypeName = StaticEnum<ECombatTeamType>()->GetNameStringByValue(static_cast<int64>(TeamType));
	for (int32 Index = 0; Index < Teams.Num(); ++Index)
	{
		const TObjectPtr<USVCombatCharacterDataAsset>& TeamAsset = Teams[Index];
		if (!TeamAsset || !TeamAsset->CharacterClass)
		{
			continue;
		}
		// 站位变换：相对模式按资产内局部变换换算为世界坐标（Local * ScenePointTransform）；绝对模式直接使用绝对变换（与场景点 GetSpawnTransforms 保持一致）
		const FTransform SpawnTransform = TeamAsset->bUseRelativeTransform
			? TeamAsset->RelativeTransform * ScenePoint->GetActorTransform()
			: TeamAsset->AbsoluteTransform;
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		// 角色命名：<配置行名>_<队伍类别>_<阵容索引>（同名冲突时引擎自动追加后缀）
		SpawnParams.Name = FName(FString::Printf(TEXT("%s_%s_%d"), *ScenePoint->GetCombatConfigRowName().ToString(), *TeamTypeName, Index));
		if (ACharacter* Character = World->SpawnActor<ACharacter>(TeamAsset->CharacterClass, SpawnTransform, SpawnParams))
		{
			// 授予战斗能力：优先走 ISVCombatAbilityGranter 扩展点（角色自定义），否则遍历 DataAsset 的 GrantedAbilities 逐个授予
			if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Character))
			{
				if (Character->Implements<USVCombatAbilityGranter>())
				{
					ISVCombatAbilityGranter::Execute_GrantCombatAbilities(Character, ASC);
				}
				else
				{
					for (const TSubclassOf<UGameplayAbility>& AbilityClass : TeamAsset->GrantedAbilities)
					{
						if (AbilityClass)
						{
							ASC->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, INDEX_NONE, Character));
						}
					}
				}
			}
			AddCharacter(TeamType, Character);
			++SpawnedCount;
		}
	}
	return SpawnedCount;
}

void USVCombatDataStore::ClearCombatData(bool bClearTeam)
{
	bInCombat = false;
	CurrentScenePoint.Reset();
	ClearAllASCData();
	ClearAllRetinues();
	if (bClearTeam)
	{
		CharacterMap.Empty();
	}
}

ASVCombatScenePoint* USVCombatDataStore::GetScenePoint() const
{
	return CurrentScenePoint.Get();
}

void USVCombatDataStore::AddCharacter(ECombatTeamType TeamType, ACharacter* Character)
{
	if (!IsValid(Character))
	{
		return;
	}

	FCombatCharacterList& Entry = CharacterMap.FindOrAdd(TeamType);
	if (Entry.Characters.AddUnique(Character) == INDEX_NONE)
	{
		return;
	}

	if (USVCombatManagerSubsystem* Owner = OwnerSubsystem.Get())
	{
		Owner->OnCombatCharacterChanged.Broadcast(true, TeamType, Character);
	}

	UE_LOG(LogCatCombatDataStore, Verbose, TEXT("AddCombatCharacter: Team=%s, Name=%s, Count=%d"),
		*UEnum::GetValueAsString(TeamType),
		*Character->GetName(), GetCharacters(TeamType).Num());
}

bool USVCombatDataStore::RemoveCharacter(ECombatTeamType TeamType, ACharacter* Character)
{
	if (!IsValid(Character))
	{
		return false;
	}

	FCombatCharacterList* Entry = CharacterMap.Find(TeamType);
	if (!Entry || Entry->Characters.Remove(Character) == 0)
	{
		return false;
	}

	if (USVCombatManagerSubsystem* Owner = OwnerSubsystem.Get())
	{
		Owner->OnCombatCharacterChanged.Broadcast(false, TeamType, Character);
	}

	UE_LOG(LogCatCombatDataStore, Verbose, TEXT("RemoveCharacter: Team=%s, Name=%s, Removed=1, RemainCount=%d"),
		*UEnum::GetValueAsString(TeamType),
		*Character->GetName(), GetCharacters(TeamType).Num());

	return true;
}

void USVCombatDataStore::ClearTeam(ECombatTeamType TeamType)
{
	FCombatCharacterList* Entry = CharacterMap.Find(TeamType);
	if (!Entry)
	{
		return;
	}

	// 先拷贝原始引用列表，再统一清空，避免边遍历边修改容器，同时把复杂度降为 O(n)
	const TArray<TWeakObjectPtr<ACharacter>> Characters = Entry->Characters;
	Entry->Characters.Empty();

	if (USVCombatManagerSubsystem* Owner = OwnerSubsystem.Get())
	{
		for (const TWeakObjectPtr<ACharacter>& WeakChar : Characters)
		{
			if (ACharacter* Character = WeakChar.Get())
			{
				Owner->OnCombatCharacterChanged.Broadcast(false, TeamType, Character);
			}
		}
	}
}

TArray<ACharacter*> USVCombatDataStore::GetCharacters(ECombatTeamType TeamType) const
{
	TArray<ACharacter*> Result;
	if (const FCombatCharacterList* Entry = CharacterMap.Find(TeamType))
	{
		Result.Reserve(Entry->Characters.Num());
		for (const TWeakObjectPtr<ACharacter>& WeakChar : Entry->Characters)
		{
			if (ACharacter* Character = WeakChar.Get())
			{
				Result.Add(Character);
			}
		}
	}
	return Result;
}

TArray<ACharacter*> USVCombatDataStore::GetAllCharacters() const
{
	TArray<ACharacter*> Result;
	for (const auto& Pair : CharacterMap)
	{
		for (const TWeakObjectPtr<ACharacter>& WeakChar : Pair.Value.Characters)
		{
			if (ACharacter* Character = WeakChar.Get())
			{
				Result.Add(Character);
			}
		}
	}
	return Result;
}

void USVCombatDataStore::SetCurrentTurnTeam(ECombatTeamType TeamType)
{
	CurrentTurnTeam = TeamType;

	// 广播队伍切换事件
	if (USVCombatManagerSubsystem* Owner = OwnerSubsystem.Get())
	{
		Owner->OnCombatTeamChanged.Broadcast(TeamType);
	}
}

void USVCombatDataStore::AdvanceToNextRound()
{
	// 推进到下一回合：切回起始阵营（先手阵营），回合数 +1
	SetCurrentTurnTeam(StartingTeam);
	IncrementRoundNumber();

	UE_LOG(LogCatCombatDataStore, Verbose, TEXT("DataStore::AdvanceToNextRound: 切换到下一回合，CurrentTurnTeam=%s, RoundNumber=%d"),
		*UEnum::GetValueAsString(CurrentTurnTeam), RoundNumber);
}

bool USVCombatDataStore::CheckCombatEnded()
{
	const ECombatResultType Result = CheckCombatResult();
	if (Result != ECombatResultType::Unsettled)
	{
		CombatResult = Result;
		UE_LOG(LogCatCombatDataStore, Log, TEXT("CheckCombatEnded: Combat ended, Result=%s"),
			*UEnum::GetValueAsString(Result));
		return true;
	}
	return false;
}

ECombatResultType USVCombatDataStore::CheckCombatResult() const
{
	if (IsTeamAllDead(ECombatTeamType::Player))
	{
		return ECombatResultType::EnemyWin;
	}
	if (IsTeamAllDead(ECombatTeamType::Enemy))
	{
		return ECombatResultType::PlayerWin;
	}
	return ECombatResultType::Unsettled;
}

bool USVCombatDataStore::IsTeamAllDead(ECombatTeamType TeamType) const
{
	const FCombatCharacterList* Entry = CharacterMap.Find(TeamType);
	if (!Entry || Entry->Characters.Num() == 0)
	{
		return true; // 空队伍视为全灭
	}

	for (const TWeakObjectPtr<ACharacter>& WeakChar : Entry->Characters)
	{
		ACharacter* Character = WeakChar.Get();
		if (!IsValid(Character))
		{
			continue;
		}

		if (Character->Implements<USVCombatCoreInterface>())
		{
			if (!ISVCombatCoreInterface::Execute_IsDeath(Character, true))
			{
				return false;
			}
		}
		else
		{
			return false; // 未实现接口视为存活
		}
	}

	return true;
}

void USVCombatDataStore::ClearAllASCData()
{
	for (ACharacter* Character : GetAllCharacters())
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Character))
		{
			// 移除所有激活中的 GameplayEffect（等价于源项目 RemoveAllBuffs）
			const FGameplayEffectQuery Query;
			const TArray<FActiveGameplayEffectHandle> ActiveEffects = ASC->GetActiveEffects(Query);
			for (const FActiveGameplayEffectHandle& Handle : ActiveEffects)
			{
				ASC->RemoveActiveGameplayEffect(Handle);
			}
		}
	}
}

void USVCombatDataStore::ClearAllRetinues()
{
	// 框架层扩展点：随从（Retinue）为源项目专属概念，此处保留空实现。
	// 新项目若需随从清理，可通过派生类或在此接入扩展接口。
}

void USVCombatDataStore::PossessMainPlayerCombatCharacter()
{
	// 通过持有者（Subsystem）获取世界
	USVCombatManagerSubsystem* Owner = OwnerSubsystem.Get();
	UWorld* World = Owner ? Owner->GetWorld() : nullptr;
	APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	if (!PC)
	{
		UE_LOG(LogCatCombatDataStore, Warning, TEXT("PossessMainPlayerCombatCharacter: 未找到玩家控制器，无法 Possess"));
		return;
	}

	ACharacter* MainPlayer = USVCombatFunctionLibrary::GetMainPlayerCombatCharacter(this);
	if (!MainPlayer)
	{
		UE_LOG(LogCatCombatDataStore, Warning, TEXT("PossessMainPlayerCombatCharacter: 未找到玩家主战斗角色，跳过"));
		return;
	}

	PC->Possess(MainPlayer);
	if (const ASVCombatScenePoint* ScenePoint = GetScenePoint())
	{
		PC->SetViewTargetWithBlend(ScenePoint->GetMainCameraActor());
	}
	UE_LOG(LogCatCombatDataStore, Verbose, TEXT("PossessMainPlayerCombatCharacter: 玩家控制器已 Possess 主战斗角色 %s"),
		*MainPlayer->GetName());
}
