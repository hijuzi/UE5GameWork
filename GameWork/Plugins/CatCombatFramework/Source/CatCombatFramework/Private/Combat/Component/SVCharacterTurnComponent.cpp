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

USVCharacterTurnComponent* USVCharacterTurnComponent::GetSVCharacterTurnComponent(const AActor* Actor)
{
	return Actor ? Actor->FindComponentByClass<USVCharacterTurnComponent>() : nullptr;
}

void USVCharacterTurnComponent::SetCoordinator(USVCombatTurnCoordinator* InCoordinator)
{
	Coordinator = InCoordinator;
}

void USVCharacterTurnComponent::RegisterCombatAbility(const FSVTurnActionData& TurnActionData, const FGameplayTag& Tag, const FGameplayAbilitySpecHandle& Handle)
{
	if (!Tag.IsValid())
	{
		return;
	}

	// 按 Role 写入对应职责映射（含重复 Tag 告警），并缓存 TurnActionData
	const auto RegisterTo = [this, &Tag, &Handle, &TurnActionData](ECombatTurnRole Role)
	{
		TMap<FGameplayTag, FGameplayAbilitySpecHandle>& HandleMap = (Role == ECombatTurnRole::Defender) ? DefenderAbilityHandles : AttackerAbilityHandles;
		TMap<FGameplayTag, FSVTurnActionData>& DataMap = (Role == ECombatTurnRole::Defender) ? DefenderAbilityData : AttackerAbilityData;

		if (HandleMap.Contains(Tag))
		{
			UE_LOG(LogCatCombatComponent, Warning, TEXT("CharacterTurnComponent: %s Tag [%s] 重复映射，跳过"),
				*GetNameSafe(GetOwner()), *Tag.ToString());
			return;
		}
		HandleMap.Add(Tag, Handle);
		DataMap.Add(Tag, TurnActionData);
	};

	switch (TurnActionData.TurnActionType)
	{
	case ESVTurnActionType::Attacker:
		RegisterTo(ECombatTurnRole::Attacker);
		break;
	case ESVTurnActionType::Defender:
		RegisterTo(ECombatTurnRole::Defender);
		break;
	case ESVTurnActionType::Both:
		RegisterTo(ECombatTurnRole::Attacker);
		RegisterTo(ECombatTurnRole::Defender);
		break;
	default:
		break;
	}
}

void USVCharacterTurnComponent::UnregisterCombatAbility(const FSVTurnActionData& TurnActionData, const FGameplayAbilitySpecHandle& Handle)
{
	// 按 Role 从对应职责映射移除该 Handle 的所有 Tag 条目，并同步清理 TurnActionData 缓存
	const auto UnregisterFrom = [this, &Handle](ECombatTurnRole Role)
	{
		TMap<FGameplayTag, FGameplayAbilitySpecHandle>& HandleMap = (Role == ECombatTurnRole::Defender) ? DefenderAbilityHandles : AttackerAbilityHandles;
		TMap<FGameplayTag, FSVTurnActionData>& DataMap = (Role == ECombatTurnRole::Defender) ? DefenderAbilityData : AttackerAbilityData;

		TArray<FGameplayTag> TagsToRemove;
		for (const auto& Pair : HandleMap)
		{
			if (Pair.Value == Handle)
			{
				TagsToRemove.Add(Pair.Key);
			}
		}
		for (const FGameplayTag& Tag : TagsToRemove)
		{
			HandleMap.Remove(Tag);
			DataMap.Remove(Tag);
		}
	};

	// 不再依赖 TurnActionData.TurnActionType 选表：该数据可能来自 CDO（OnRemoveAbility 在授予上下文执行时的默认值），
	// 与 AbilitySet override 写入主实例的最终 TurnActionType 不一致，按类型选表会查错表导致映射泄漏。
	// Handle 全局唯一，只会出现在实际注册过的那张表，故对两张表统一清理无副作用。
	UnregisterFrom(ECombatTurnRole::Attacker);
	UnregisterFrom(ECombatTurnRole::Defender);
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

	// 通过 Tag 拿到 GA 实例（优先运行时实例，fallback 到 CDO）
	const UGameplayAbility* Ability = GetAbilityInstanceByTag(Tag, Role);
	if (!Ability)
	{
		return false;
	}

	const UAbilitySystemComponent* ASC = GetCachedASC();
	if (!ASC)
	{
		return false;
	}

	// 限制次数类型：剩余次数 ≤ 0 时不可再激活
	int32 RemainingCount = 0;
	if (GetRemainingActivationCount(Tag, Role, RemainingCount) && RemainingCount <= 0)
	{
		return false;
	}

	// 从 Handle 映射取有效 Handle（CDO 上 GetCurrentAbilitySpecHandle 无效，不能用）
	const TMap<FGameplayTag, FGameplayAbilitySpecHandle>& HandleMap =
		(Role == ECombatTurnRole::Defender) ? DefenderAbilityHandles : AttackerAbilityHandles;
	const FGameplayAbilitySpecHandle* Found = HandleMap.Find(Tag);
	if (!Found || !Found->IsValid())
	{
		return false;
	}

	// 调用 Ability 的无副作用 CanActivateAbility 判断
	const FGameplayAbilityActorInfo* ActorInfo = ASC->AbilityActorInfo.Get();
	return Ability->CanActivateAbility(*Found, ActorInfo);
}

TMap<FGameplayTag, int32> USVCharacterTurnComponent::GetAllActivationCounts() const
{
	// 合并三个分类计数（PerTurn + WholeBattle + Unlimited），供调试展示
	TMap<FGameplayTag, int32> Result = PerTurnActivationCounts;
	Result.Append(BattleActivationCounts);
	Result.Append(UnlimitedActivationCounts);
	return Result;
}

const FSVTurnActionData* USVCharacterTurnComponent::FindAbilityData(const FGameplayTag& Tag, ECombatTurnRole Role) const
{
	const TMap<FGameplayTag, FSVTurnActionData>& DataMap = (Role == ECombatTurnRole::Defender) ? DefenderAbilityData : AttackerAbilityData;
	return DataMap.Find(Tag);
}

const TMap<FGameplayTag, int32>* USVCharacterTurnComponent::GetCountMapForTag(const FGameplayTag& Tag, ECombatTurnRole Role) const
{
	const FSVTurnActionData* Data = FindAbilityData(Tag, Role);
	if (!Data || Data->LimitType == ESVActivationLimitType::IgnoreLimit)
	{
		return nullptr;  // 未注册 / IgnoreLimit（忽略限制）：不记录
	}
	if (Data->LimitType == ESVActivationLimitType::Limited)
	{
		return (Data->LimitScope == ESVActivationLimitScope::PerTurn) ? &PerTurnActivationCounts : &BattleActivationCounts;
	}
	return &UnlimitedActivationCounts;  // Unlimited（无限次数）
}

TMap<FGameplayTag, int32>* USVCharacterTurnComponent::GetCountMapForTag(const FGameplayTag& Tag, ECombatTurnRole Role)
{
	const FSVTurnActionData* Data = FindAbilityData(Tag, Role);
	if (!Data || Data->LimitType == ESVActivationLimitType::IgnoreLimit)
	{
		return nullptr;  // 未注册 / IgnoreLimit（忽略限制）：不记录
	}
	if (Data->LimitType == ESVActivationLimitType::Limited)
	{
		return (Data->LimitScope == ESVActivationLimitScope::PerTurn) ? &PerTurnActivationCounts : &BattleActivationCounts;
	}
	return &UnlimitedActivationCounts;  // Unlimited（无限次数）
}

int32 USVCharacterTurnComponent::GetActivationCount(const FGameplayTag& Tag, ECombatTurnRole Role) const
{
	const TMap<FGameplayTag, int32>* CountMap = GetCountMapForTag(Tag, Role);
	if (!CountMap)
	{
		return 0;
	}
	const int32* Found = CountMap->Find(Tag);
	return Found ? *Found : 0;
}

void USVCharacterTurnComponent::IncrementActivationCount(const FGameplayTag& Tag, ECombatTurnRole Role)
{
	if (!Tag.IsValid())
	{
		return;
	}

	TMap<FGameplayTag, int32>* CountMap = GetCountMapForTag(Tag, Role);
	if (!CountMap)
	{
		return;  // 未注册 / IgnoreLimit（忽略限制）：不记录激活次数
	}

	++CountMap->FindOrAdd(Tag);
}

void USVCharacterTurnComponent::ResetActivationCount(const FGameplayTag& Tag)
{
	PerTurnActivationCounts.Remove(Tag);
	BattleActivationCounts.Remove(Tag);
	UnlimitedActivationCounts.Remove(Tag);
}

void USVCharacterTurnComponent::ResetAllActivationCounts()
{
	PerTurnActivationCounts.Empty();
	BattleActivationCounts.Empty();
	UnlimitedActivationCounts.Empty();
}

void USVCharacterTurnComponent::ResetPerTurnActivationCounts()
{
	// PerTurn 分类已独立成 map，直接清空即可，无需遍历判断
	PerTurnActivationCounts.Empty();
}

bool USVCharacterTurnComponent::GetRemainingActivationCount(const FGameplayTag& Tag, ECombatTurnRole Role, int32& OutRemainingCount) const
{
	// 直接从注册时缓存的 TurnActionData 读取限制配置（不再依赖能力实例/CDO）
	const FSVTurnActionData* Data = FindAbilityData(Tag, Role);
	if (!Data || Data->LimitType != ESVActivationLimitType::Limited)
	{
		return false;
	}

	const int32 Activated = GetActivationCount(Tag, Role);
	OutRemainingCount = Data->LimitCount - Activated;
	return true;
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

bool USVCharacterTurnComponent::ShouldEnforceActionSlotLimit() const
{
	// 槽位收敛为通用能力：只要配置了 MaxActionCountPerTurn（>0）即启用；
	// 约束在攻击方职责上（防守方为被动响应，无行动槽位概念）
	return TurnRole == ECombatTurnRole::Attacker && MaxActionCountPerTurn > 0;
}

FGameplayTagContainer USVCharacterTurnComponent::GetRemainingActionTags(bool bAllowRepeats) const
{
	const TArray<FGameplayTag> RoleTags = GetRoleTags();

	// 1) 限制次数（Limited）技能用尽后的收尾（仅针对技能自身的 bEndTurnWhenLimitReached）：
	//    标记「用尽即收尾」的技能一旦次数用尽 → 整组返回空（立即结束当前回合）；
	//    未标记的技能用尽只移除自身（激活次数 > 0，不会再进入下方「可发起候选」），不在此处结束回合。
	//    注：每回合行动次数上限由 MaxActionCountPerTurn 独立控制（见 OnEnterDecisionMaking），两者互不耦合。
	for (const FGameplayTag& Tag : RoleTags)
	{
		if (!Tag.IsValid())
		{
			continue;
		}

		const FSVTurnActionData* Data = FindAbilityData(Tag, TurnRole);
		if (!Data || Data->LimitType != ESVActivationLimitType::Limited)
		{
			continue;
		}

		int32 RemainingCount = 0;
		if (GetRemainingActivationCount(Tag, TurnRole, RemainingCount) && RemainingCount <= 0)
		{
			// 次数用尽且该能力声明「用尽即收尾」：立即结束当前回合
			if (Data->bEndTurnWhenLimitReached)
			{
				return FGameplayTagContainer();
			}
		}
	}

	// 2) 收集可发起的行动能力：非忽略限制（Limited / Unlimited）才进入候选。
	//    未用过（激活次数为 0）默认即可发起；bAllowRepeats=true 时已用过但仍有剩余额度的也可重复进入（Unlimited 恒有额度 / Limited 剩余 > 0）
	FGameplayTagContainer ActivatableTags;
	for (const FGameplayTag& Tag : RoleTags)
	{
		if (!Tag.IsValid())
		{
			continue;
		}

		const FSVTurnActionData* Data = FindAbilityData(Tag, TurnRole);
		if (!Data || Data->LimitType == ESVActivationLimitType::IgnoreLimit)
		{
			continue;  // 忽略限制不进入本次可激活候选
		}

		// 是否可进入候选：
		// - 未用过（激活次数 0）：默认即可发起；
		// - 已用过：需 bAllowRepeats=true 且仍有剩余额度（Unlimited 恒有额度 / Limited 剩余次数 > 0），否则不可重复进入
		const int32 ActivatedCount = GetActivationCount(Tag, TurnRole);
		const bool bCanInitiate = (ActivatedCount == 0)
			|| (bAllowRepeats && (Data->LimitType == ESVActivationLimitType::Unlimited || Data->LimitCount - ActivatedCount > 0));
		if (bCanInitiate)
		{
			ActivatableTags.AddTag(Tag);
		}
	}

	return ActivatableTags;
}

void USVCharacterTurnComponent::OnEnterDecisionMaking()
{
	// 槽位收敛（攻击方职责 + 已配置行动槽位上限）：本回合行动次数已达上限，直接结束当前回合（延迟到下一帧，避免状态迁移重入）
	if (ShouldEnforceActionSlotLimit() && ActionCount >= MaxActionCountPerTurn)
	{
		const FString OwnerName = GetNameSafe(GetOwner());
		UE_LOG(LogCatCombatComponent, Verbose, TEXT("CharacterTurnComponent: %s 行动槽位已用尽 (%d/%d)，结束当前回合"),
			*OwnerName, ActionCount, MaxActionCountPerTurn);

		ScheduleTurnFinished();
		return;
	}

	// 计算本回合剩余可执行的行动能力 Tags（为空即全部执行完，结束当前回合）
	const FGameplayTagContainer ActivatableTagContainer = GetRemainingActionTags(true);

	// 仅临时日志，不存成员
	const FString ActivatableTagsStr = ActivatableTagContainer.ToStringSimple();
	const FString OwnerName = GetNameSafe(GetOwner());
	UE_LOG(LogCatCombatComponent, Verbose, TEXT("CharacterTurnComponent: %s 进入决策阶段 [%s]，可激活 Tags=[%s]"),
		*OwnerName, *UEnum::GetValueAsString(State), *ActivatableTagsStr);

	// 没有可激活的能力：调度结束当前回合（延迟到下一帧执行，避免在 SetState 状态迁移过程中再次触发状态迁移的重入）
	if (ActivatableTagContainer.Num() == 0)
	{
		UE_LOG(LogCatCombatComponent, Verbose, TEXT("CharacterTurnComponent: %s 无可激活能力，结束当前回合"),
			*OwnerName);

		ScheduleTurnFinished();
		return;
	}

	// 有可激活能力：交由外部桥接层（输入/ AI 决策桥）按需挑选并激活对应 GA，
	// 组件保持纯状态机不感知具体角色类型；激活后由能力通过 NotifyActionStarted / NotifyActionFinished 回写状态。
}

void USVCharacterTurnComponent::OnEnterActed()
{
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

	// 回合开始：重置行动次数
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
	// 按 TurnRole 从对应映射的 keys 生成「应该激活的 Tags」数组
	TArray<FGameplayTag> Tags;
	if (TurnRole == ECombatTurnRole::Attacker)
	{
		AttackerAbilityHandles.GetKeys(Tags);
	}
	else if (TurnRole == ECombatTurnRole::Defender)
	{
		DefenderAbilityHandles.GetKeys(Tags);
	}

	// TurnRole 无效（None）时返回空数组
	return Tags;
}

TArray<FGameplayTag> USVCharacterTurnComponent::GetAttackerTags() const
{
	TArray<FGameplayTag> Tags;
	AttackerAbilityHandles.GetKeys(Tags);
	return Tags;
}

TArray<FGameplayTag> USVCharacterTurnComponent::GetDefenderTags() const
{
	TArray<FGameplayTag> Tags;
	DefenderAbilityHandles.GetKeys(Tags);
	return Tags;
}

UGameplayAbility* USVCharacterTurnComponent::GetAbilityInstanceByTag(const FGameplayTag& Tag, ECombatTurnRole Role) const
{
	const TMap<FGameplayTag, FGameplayAbilitySpecHandle>& HandleMap = (Role == ECombatTurnRole::Defender) ? DefenderAbilityHandles : AttackerAbilityHandles;

	const FGameplayAbilitySpecHandle* Found = HandleMap.Find(Tag);
	if (!Found || !Found->IsValid())
	{
		return nullptr;
	}

	const UAbilitySystemComponent* ASC = GetCachedASC();
	if (!ASC)
	{
		return nullptr;
	}

	// 优先取运行时实例，fallback 到 Spec.Ability（CDO）
	const FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(*Found);
	if (Spec && Spec->GetPrimaryInstance())
	{
		return Spec->GetPrimaryInstance();
	}

	return (Spec && Spec->Ability) ? Spec->Ability : nullptr;
}

bool USVCharacterTurnComponent::TryActivateActionByTag(const FGameplayTag& Tag, ECombatTurnRole Role)
{
	if (!Tag.IsValid())
	{
		return false;
	}

	const TMap<FGameplayTag, FGameplayAbilitySpecHandle>& HandleMap = (Role == ECombatTurnRole::Defender) ? DefenderAbilityHandles : AttackerAbilityHandles;
	const FGameplayAbilitySpecHandle* Found = HandleMap.Find(Tag);
	if (!Found || !Found->IsValid())
	{
		return false;
	}

	UAbilitySystemComponent* ASC = GetCachedASC();
	if (!ASC)
	{
		return false;
	}

	return ASC->TryActivateAbility(*Found);
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
	// 记录当前正在 Acting 的能力 tag（行动结束移除）
	CurrentActingTag = FindRoleTag(AssetTags);

	// 该能力激活次数 +1（按当前职责查配置归入 PerTurn/WholeBattle/Unlimited 计数）
	if (CurrentActingTag.IsValid())
	{
		IncrementActivationCount(CurrentActingTag, TurnRole);
	}

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

bool USVCharacterTurnComponent::IsCoordinatorFinishing() const
{
	const USVCombatTurnCoordinator* Coord = Coordinator.Get();
	return Coord && Coord->GetPhase() == ECombatCampPhase::Finishing;
}

void USVCharacterTurnComponent::NotifyActionFinished(const FGameplayTagContainer& AssetTags)
{
	// 清除当前正在 Acting 的能力 tag（先记录被清除的 tag，便于排查行动链）
	const FGameplayTag FinishedActionTag = CurrentActingTag;
	CurrentActingTag = FGameplayTag();
	UE_LOG(LogCatCombatComponent, Log, TEXT("CharacterTurnComponent::NotifyActionFinished: %s 清除 ActingTag=%s"),
		*GetNameSafe(GetOwner()), FinishedActionTag.IsValid() ? *FinishedActionTag.ToString() : TEXT("None"));

	// 决定行动结束后的归宿：
	// - 协调器处于「行动收尾（Finishing）」阶段：直接结束本角色回合（Acted，经 SetState 钩子上报协调器继续收尾），不再回到决策/防守等待；
	// - 否则按职责：Attacker -> Selecting（等待选择/触发下一个 Action），Defender -> Defending（反击等被动行为结束还原防守状态），None -> Idle
	ECharacterTurnState NextState;
	if (IsCoordinatorFinishing())
	{
		NextState = ECharacterTurnState::Acted;
	}
	else
	{
		switch (TurnRole)
		{
		case ECombatTurnRole::Attacker:
			NextState = ECharacterTurnState::Selecting;
			break;
		case ECombatTurnRole::Defender:
			NextState = ECharacterTurnState::Defending;
			break;
		default:
			NextState = ECharacterTurnState::Idle;
			break;
		}
	}

	SetState(NextState);

	const FString AssetTagsStr = AssetTags.ToStringSimple();
	const FString OwnerName = GetNameSafe(GetOwner());
	UE_LOG(LogCatCombatComponent, Verbose, TEXT("CharacterTurnComponent::NotifyActionFinished: %s, AssetTags=%s, State %s -> %s"),
		*OwnerName, *AssetTagsStr, *UEnum::GetValueAsString(PreviousState), *UEnum::GetValueAsString(State));
}

void USVCharacterTurnComponent::ScheduleTurnFinished()
{
	// 结束当前回合的收敛动作：优先延迟到下一帧执行（避免在 SetState 状态迁移过程中再次触发状态迁移的重入）
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
