// Fill out your copyright notice in the Description page of Project Settings.

#include "Combat/Component/SVCharacterTurnComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Combat/SVCombatManagerSubsystem.h"
#include "Combat/Turn/SVCombatTurnCoordinator.h"
#include "Engine/GameInstance.h"
#include "GameFramework/Character.h"
#include "CatCombatGameplayTags.h"
#include "CatCombatLog.h"
#include "TimerManager.h"

USVCharacterTurnComponent::USVCharacterTurnComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USVCharacterTurnComponent::SetCoordinator(USVCombatTurnCoordinator* InCoordinator)
{
	Coordinator = InCoordinator;
}

void USVCharacterTurnComponent::SetCombatTags(const TArray<FGameplayTag>& InAttackerTags, const TArray<FGameplayTag>& InDefenderTags)
{
	AttackerTagsConfig = InAttackerTags;
	DefenderTagsConfig = InDefenderTags;

	// 归集攻击方/防守方能力 SpecHandle（初始化一次，建立 Tag → Handle 映射）
	AttackerAbilityHandles.Reset();
	DefenderAbilityHandles.Reset();
	if (UAbilitySystemComponent* ASC = GetCachedASC())
	{
		for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
		{
			if (!Spec.Ability)
			{
				continue;
			}
			const FGameplayTagContainer AssetTags = Spec.Ability->GetAssetTags();

			// 攻击方：遍历配置 tag，命中 AssetTags 则建立映射
			for (const FGameplayTag& ConfigTag : AttackerTagsConfig)
			{
				if (ConfigTag.IsValid() && AssetTags.HasTagExact(ConfigTag))
				{
					if (AttackerAbilityHandles.Contains(ConfigTag))
					{
						UE_LOG(LogCatCombatComponent, Warning, TEXT("CharacterTurnComponent: %s 攻击方 Tag [%s] 重复映射，跳过后续技能"),
							*GetNameSafe(GetOwner()), *ConfigTag.ToString());
						continue;
					}
					AttackerAbilityHandles.Add(ConfigTag, Spec.Handle);
				}
			}
			// 防守方
			for (const FGameplayTag& ConfigTag : DefenderTagsConfig)
			{
				if (ConfigTag.IsValid() && AssetTags.HasTagExact(ConfigTag))
				{
					if (DefenderAbilityHandles.Contains(ConfigTag))
					{
						UE_LOG(LogCatCombatComponent, Warning, TEXT("CharacterTurnComponent: %s 防守方 Tag [%s] 重复映射，跳过后续技能"),
							*GetNameSafe(GetOwner()), *ConfigTag.ToString());
						continue;
					}
					DefenderAbilityHandles.Add(ConfigTag, Spec.Handle);
				}
			}
		}
	}

	const FString OwnerName = GetNameSafe(GetOwner());
	UE_LOG(LogCatCombatComponent, Log, TEXT("CharacterTurnComponent: %s 战斗能力标签配置已初始化（攻击方 %d/%d，防守方 %d/%d）"),
		*OwnerName, AttackerAbilityHandles.Num(), AttackerTagsConfig.Num(), DefenderAbilityHandles.Num(), DefenderTagsConfig.Num());
}

void USVCharacterTurnComponent::SetState(ECharacterTurnState NewState)
{
	if (State == NewState)
	{
		return;
	}
	// 记录上一个阶段（调试/观测用）
	PreviousState = State;
	State = NewState;
	const FString OldStateStr = UEnum::GetValueAsString(PreviousState);
	const FString NewStateStr = UEnum::GetValueAsString(NewState);
	const FString OwnerName = GetNameSafe(GetOwner());
	UE_LOG(LogCatCombatComponent, Verbose, TEXT("CharacterTurnComponent: %s 状态迁移 %s -> %s"),
		*OwnerName, *OldStateStr, *NewStateStr);

	// 将状态 Tag 挂到角色 ASC（移除旧、添加新）
	ApplyStateTagToASC();

	// 进入状态的迁移钩子
	switch (NewState)
	{
	case ECharacterTurnState::Selecting:
		OnEnterDecisionMaking();
		break;
	case ECharacterTurnState::Defending:
		OnEnterDecisionMaking();
		break;
	case ECharacterTurnState::Deferred:
		OnEnterDeferred();
		break;
	case ECharacterTurnState::Acted:
		OnEnterActed();
		break;

	default:
		break;
	}
}

FGameplayTag USVCharacterTurnComponent::GetStateTag(ECharacterTurnState InState)
{
	switch (InState)
	{
	case ECharacterTurnState::Idle:      return CatCombatGameplayTags::TAG_STATE_TURN_IDLE;
	case ECharacterTurnState::Selecting: return CatCombatGameplayTags::TAG_STATE_TURN_SELECTING;
	case ECharacterTurnState::Acting:    return CatCombatGameplayTags::TAG_STATE_TURN_ACTING;
	case ECharacterTurnState::Defending: return CatCombatGameplayTags::TAG_STATE_TURN_DEFENDING;
	case ECharacterTurnState::Deferred:  return CatCombatGameplayTags::TAG_STATE_TURN_DEFERRED;
	case ECharacterTurnState::Acted:     return CatCombatGameplayTags::TAG_STATE_TURN_ACTED;
	default:                             return FGameplayTag();
	}
}

UAbilitySystemComponent* USVCharacterTurnComponent::GetCachedASC() const
{
	// 缓存有效则直接返回，失效（未初始化/已销毁）则重新查找
	if (!CachedASC.IsValid())
	{
		CachedASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	}
	return CachedASC.Get();
}

bool USVCharacterTurnComponent::CanActivateAbilityByTag(const FGameplayTag& Tag, ECombatTurnRole Role) const
{
	if (!Tag.IsValid())
	{
		return false;
	}

	// 按传入职责选择 Handle 缓存
	const TMap<FGameplayTag, FGameplayAbilitySpecHandle>& HandleMap =
		(Role == ECombatTurnRole::Defender) ? DefenderAbilityHandles : AttackerAbilityHandles;

	const FGameplayAbilitySpecHandle* Found = HandleMap.Find(Tag);
	if (!Found || !Found->IsValid())
	{
		return false;
	}

	const UAbilitySystemComponent* ASC = GetCachedASC();
	if (!ASC)
	{
		return false;
	}

	// 通过 Handle 找到 Spec，调用 Ability 的无副作用 CanActivateAbility 判断
	const FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(*Found);
	if (!Spec || !Spec->Ability)
	{
		return false;
	}

	const FGameplayAbilityActorInfo* ActorInfo = ASC->AbilityActorInfo.Get();
	return Spec->Ability->CanActivateAbility(*Found, ActorInfo);
}

UClass* USVCharacterTurnComponent::GetCurrentActingAbilityClass() const
{
	if (!CurrentActingTag.IsValid())
	{
		return nullptr;
	}

	// 按当前职责选择 Handle 缓存
	const TMap<FGameplayTag, FGameplayAbilitySpecHandle>& HandleMap =
		(TurnRole == ECombatTurnRole::Defender) ? DefenderAbilityHandles : AttackerAbilityHandles;

	const FGameplayAbilitySpecHandle* Found = HandleMap.Find(CurrentActingTag);
	if (!Found || !Found->IsValid())
	{
		return nullptr;
	}

	const UAbilitySystemComponent* ASC = GetCachedASC();
	if (!ASC)
	{
		return nullptr;
	}

	const FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(*Found);
	if (!Spec || !Spec->Ability)
	{
		return nullptr;
	}

	return Spec->Ability->GetClass();
}

void USVCharacterTurnComponent::ApplyStateTagToASC()
{
	UAbilitySystemComponent* ASC = GetCachedASC();
	if (!ASC)
	{
		return;
	}

	const FGameplayTag NewStateTag = GetStateTag(State);

	// 幂等保护：目标状态 Tag 与当前已挂的一致，无需操作
	if (NewStateTag == CurrentAppliedStateTag)
	{
		return;
	}

	// 只移除上一个已挂的状态 Tag（O(1)，精确匹配）
	if (CurrentAppliedStateTag.IsValid())
	{
		ASC->RemoveLooseGameplayTag(CurrentAppliedStateTag);
	}

	// 添加新的状态 Tag，并记住
	CurrentAppliedStateTag = NewStateTag;
	if (CurrentAppliedStateTag.IsValid())
	{
		ASC->AddLooseGameplayTag(CurrentAppliedStateTag);
	}
}

void USVCharacterTurnComponent::OnEnterDecisionMaking()
{
	// 计算「还有哪些未激活的 tags」：GetRoleTags 中排除已激活的（用 Container 承载，便于后续 O(1) 命中检测）
	FGameplayTagContainer RemainingTagContainer;
	for (const FGameplayTag& Tag : GetRoleTags())
	{
		if (Tag.IsValid() && !ActiveActionTags.Contains(Tag))
		{
			RemainingTagContainer.AddTag(Tag);
		}
	}

	// 查询「还有哪些可以激活」：用 RemainingTags 里的 tag 直接查缓存的 Tag → Handle 映射（按 TurnRole 选择攻击/防守）。
	UAbilitySystemComponent* ASC = GetCachedASC();
	FGameplayTagContainer ActivatableTagContainer;
	if (ASC)
	{
		const FGameplayAbilityActorInfo* ActorInfo = ASC->AbilityActorInfo.Get();
		const TMap<FGameplayTag, FGameplayAbilitySpecHandle>& HandleMap =
			(TurnRole == ECombatTurnRole::Defender) ? DefenderAbilityHandles : AttackerAbilityHandles;

		for (const FGameplayTag& Tag : RemainingTagContainer)
		{
			const FGameplayAbilitySpecHandle* HandlePtr = HandleMap.Find(Tag);
			if (!HandlePtr)
			{
				continue;
			}
			const FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(*HandlePtr);
			if (Spec && Spec->Ability && Spec->Ability->CanActivateAbility(*HandlePtr, ActorInfo))
			{
				ActivatableTagContainer.AddTag(Tag);
			}
		}
	}

	// 仅临时日志，不存成员
	const FString RemainingTagsStr = RemainingTagContainer.ToStringSimple();
	const FString ActivatableTagsStr = ActivatableTagContainer.ToStringSimple();
	const FString OwnerName = GetNameSafe(GetOwner());
	UE_LOG(LogCatCombatComponent, Verbose, TEXT("CharacterTurnComponent: %s 进入决策阶段 [%s]，剩余未激活 Tags=[%s]，可激活 Tags=[%s]"),
		*OwnerName, *UEnum::GetValueAsString(State), *RemainingTagsStr, *ActivatableTagsStr);

	// 没有可激活的能力：延迟到下一帧结束当前回合（避免在 SetState 状态迁移过程中再次触发状态迁移的重入）
	if (ActivatableTagContainer.Num() == 0)
	{
		UE_LOG(LogCatCombatComponent, Verbose, TEXT("CharacterTurnComponent: %s 无可激活能力，结束当前回合"),
			*OwnerName);

		UWorld* World = GetOwner() ? GetOwner()->GetWorld() : nullptr;
		if (World)
		{
			World->GetTimerManager().SetTimerForNextTick([this]()
			{
				NotifyTurnFinished();
			});
		}
		else
		{
			// World 无效时兜底直接上报（极端情况，如角色即将销毁）
			NotifyTurnFinished();
		}
	}
}

void USVCharacterTurnComponent::OnEnterActed()
{
	// 回合结束：清空激活缓存
	ActiveActionTags.Empty();

	// 结束上报协调器（从 PendingReadySet 移除角色）
	if (USVCombatTurnCoordinator* Coord = Coordinator.Get())
	{
		Coord->NotifyTurnFinished(GetOwner<ACharacter>());
	}
}

void USVCharacterTurnComponent::OnEnterDeferred()
{
	// 中断/挂起：不清空激活缓存，仅上报协调器（从 PendingReadySet 移除角色）
	if (USVCombatTurnCoordinator* Coord = Coordinator.Get())
	{
		Coord->NotifyTurnFinished(GetOwner<ACharacter>());
	}
}

void USVCharacterTurnComponent::HandleBeginTurn(ECombatTurnRole InRole)
{
	TurnRole = InRole;

	// 回合开始：清空上一回合的激活缓存，重置行动次数
	ActiveActionTags.Empty();
	ResetActionCount();

	// 惰性注入协调器：通过 GameInstance → CombatManagerSubsystem → TurnCoordinator 反向查找，
	// 保持协调器不感知组件类型（延续 ISVCombatCoreInterface 接口解耦）。
	if (!Coordinator.IsValid())
	{
		if (const UGameInstance* GI = GetOwner()->GetGameInstance())
		{
			if (USVCombatManagerSubsystem* Subsystem = GI->GetSubsystem<USVCombatManagerSubsystem>())
			{
				Coordinator = Subsystem->GetTurnCoordinator();
			}
		}
	}

	if (InRole == ECombatTurnRole::Attacker)
	{
		SetState(ECharacterTurnState::Selecting);
	}
	else if (InRole == ECombatTurnRole::Defender)
	{
		SetState(ECharacterTurnState::Defending);
	}
	else
	{
		SetState(ECharacterTurnState::Idle);
	}
}

TArray<FGameplayTag> USVCharacterTurnComponent::GetRoleTags() const
{
	// 按 TurnRole 返回对应的「应该激活的 Tags」配置数组
	if (TurnRole == ECombatTurnRole::Attacker)
	{
		return AttackerTagsConfig;
	}
	if (TurnRole == ECombatTurnRole::Defender)
	{
		return DefenderTagsConfig;
	}

	// TurnRole 无效（None）时返回空数组
	return TArray<FGameplayTag>();
}

void USVCharacterTurnComponent::AddActiveActionTags(const FGameplayTagContainer& AssetTags)
{
	// 精确匹配：AssetTags 命中「当前 TurnRole 应该激活的 Tags」时，去重添加到激活缓存
	for (const FGameplayTag& ConfigTag : GetRoleTags())
	{
		if (ConfigTag.IsValid() && AssetTags.HasTagExact(ConfigTag))
		{
			ActiveActionTags.AddUnique(ConfigTag);
		}
	}
}

FGameplayTag USVCharacterTurnComponent::FindRoleTag(const FGameplayTagContainer& AssetTags) const
{
	// 精确匹配：AssetTags 命中「当前 TurnRole 应该激活的 Tags」时，返回第一个命中的配置 tag
	for (const FGameplayTag& ConfigTag : GetRoleTags())
	{
		if (ConfigTag.IsValid() && AssetTags.HasTagExact(ConfigTag))
		{
			return ConfigTag;
		}
	}
	return FGameplayTag();
}

void USVCharacterTurnComponent::NotifyActionStarted(const FGameplayTagContainer& AssetTags)
{
	// 缓存激活的能力 tag（按 TurnRole 精确匹配配置数组）
	AddActiveActionTags(AssetTags);

	// 记录当前正在 Acting 的能力 tag（行动结束移除）
	CurrentActingTag = FindRoleTag(AssetTags);

	// 行动次数 +1（本回合已发起的行动次数）
	IncrementActionCount();

	// 行动开始：进入 Acting 阶段
	SetState(ECharacterTurnState::Acting);

	// 转发协调器（对称于 NotifyActionFinished 语义，供全局观测）
	if (USVCombatTurnCoordinator* Coord = Coordinator.Get())
	{
		Coord->NotifyActionStarted(GetOwner<ACharacter>());
	}

	const FString AssetTagsStr = AssetTags.ToStringSimple();
	const FString OwnerName = GetNameSafe(GetOwner());
	UE_LOG(LogCatCombatComponent, Verbose, TEXT("CharacterTurnComponent::NotifyActionStarted: %s, AssetTags=%s"),
		*OwnerName, *AssetTagsStr);
}

void USVCharacterTurnComponent::NotifyActionFinished(bool bInterrupted, const FGameplayTagContainer& AssetTags)
{
	// 缓存激活的能力 tag（按 TurnRole 精确匹配配置数组）
	AddActiveActionTags(AssetTags);

	// 清除当前正在 Acting 的能力 tag
	CurrentActingTag = FGameplayTag();

	// 单个 Action（GA）结束：按职责决定动作结束后的归宿
	// - Attacker：回到 Selecting，等待选择/触发下一个 Action（全部结束由 NotifyTurnFinished 上报）
	// - Defender：回到 Defending（反击等被动行为结束后还原防守状态）
	// - None：回到 Idle
	// bInterrupted 暂保留（后续真正「被外部打断」场景接入时使用，当前不驱动状态）

	switch (TurnRole)
	{
	case ECombatTurnRole::Attacker:
		SetState(ECharacterTurnState::Selecting);
		break;
	case ECombatTurnRole::Defender:
		SetState(ECharacterTurnState::Defending);
		break;
	default:
		SetState(ECharacterTurnState::Idle);
		break;
	}

	const FString AssetTagsStr = AssetTags.ToStringSimple();
	const FString OwnerName = GetNameSafe(GetOwner());
	UE_LOG(LogCatCombatComponent, Verbose, TEXT("CharacterTurnComponent::NotifyActionFinished: %s, bInterrupted=%d, AssetTags=%s"),
		*OwnerName, bInterrupted, *AssetTagsStr);
}

void USVCharacterTurnComponent::NotifyTurnFinished()
{
	// 上报协调器由 OnEnterActed 状态迁移钩子统一处理
	SetState(ECharacterTurnState::Acted);
}

void USVCharacterTurnComponent::NotifyDeferred()
{
	// 上报协调器由 OnEnterDeferred 状态迁移钩子统一处理
	SetState(ECharacterTurnState::Deferred);
}

void USVCharacterTurnComponent::TryReactiveDefense(const FReactiveDefenseRequest& Request, FReactiveDefenseResult& Out)
{
	// 抽象扩展点：具体防御行为（闪避/格挡等）通过「防御类型 → 处理节点」注册扩展，第 5 步接入
	Out.bSucceeded = false;
}

void USVCharacterTurnComponent::NotifyHitResolve(const FHitResolveContext& Context)
{
	// 抽象扩展点：受击结算时刻钩子，反击等行为第 5 步接入
}
