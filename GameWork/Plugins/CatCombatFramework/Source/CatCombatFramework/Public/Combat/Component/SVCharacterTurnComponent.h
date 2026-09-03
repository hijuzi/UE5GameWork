// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "GameplayAbilitySpecHandle.h"
#include "Combat/SVCombatTypes.h"
#include "SVCharacterTurnComponent.generated.h"

class USVCombatTurnCoordinator;
class UAbilitySystemComponent;

/**
 * 角色级回合组件（决策层，自驱动状态机）。
 * 挂载在战斗角色上，持有角色自身回合阶段与跨回合行动队列。
 *
 * 不决定"下一个轮到谁"（那是全局协调器的职责），只决定"我此刻做什么、何时上报完成"。
 * 由角色类的 ISVCombatCoreInterface::OnBeginTurn_Implementation 转发调用 HandleBeginTurn。
 */
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class CATCOMBATFRAMEWORK_API USVCharacterTurnComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USVCharacterTurnComponent();

	/** 响应协调器的回合通知：下发本回合职责（Actor/Defender） */
	void HandleBeginTurn(ECombatTurnRole InRole);

	/** 获取当前角色阶段 */
	ECharacterTurnState GetState() const { return State; }

	/** 获取上一个角色阶段（调试/观测用，未发生迁移时为 Idle） */
	ECharacterTurnState GetPreviousState() const { return PreviousState; }

	/** 获取当前回合职责 */
	ECombatTurnRole GetTurnRole() const { return TurnRole; }

	/** 获取跨回合待办队列（只读，供调试/观测） */
	const TArray<FPendingTurnAction>& GetPendingActions() const { return PendingActions; }

	/** 设置所属协调器（弱引用，由战斗框架在初始化时注入） */
	void SetCoordinator(USVCombatTurnCoordinator* InCoordinator);

	/** 初始化战斗能力标签：从 AbilitySet 拷贝攻击方/防守方能力 Tags（在 InitAbilityActorInfo 时调用） */
	void SetCombatTags(const TArray<FGameplayTag>& InAttackerTags, const TArray<FGameplayTag>& InDefenderTags);

	/** 获取攻击方能力 Tags（只读，配置数据，运行期不可修改） */
	const TArray<FGameplayTag>& GetAttackerTags() const { return AttackerTagsConfig; }

	/** 获取防守方能力 Tags（只读，配置数据，运行期不可修改） */
	const TArray<FGameplayTag>& GetDefenderTags() const { return DefenderTagsConfig; }

	/** 获取已激活的行动 Tags 缓存（只读，供调试/查询） */
	const TArray<FGameplayTag>& GetActiveActionTags() const { return ActiveActionTags; }

	/** 判断指定 Tag 是否已激活（存在于 ActiveActionTags 中，只读） */
	bool IsActionActivated(const FGameplayTag& Tag) const { return ActiveActionTags.Contains(Tag); }

	/** 获取当前正在 Acting 的能力 Tag（无则为空 Tag，只读，供调试/观测） */
	FGameplayTag GetCurrentActingTag() const { return CurrentActingTag; }

	/** 获取当前正在 Acting 的能力对应的 GA Class（用于调试显示，无则返回 nullptr） */
	UClass* GetCurrentActingAbilityClass() const;

	/** 判断指定 Tag 对应的能力当前能否激活（按 Role 选 Handle Map，调用 ASC 原生 CanActivateAbility） */
	bool CanActivateAbilityByTag(const FGameplayTag& Tag, ECombatTurnRole Role) const;

	/** 获取本回合已发起的行动次数（每回合清零，只读，供调试/查询） */
	int32 GetActionCount() const { return ActionCount; }

	/** 行动次数 +1（封装递增逻辑） */
	void IncrementActionCount() { ++ActionCount; }

	/** 重置行动次数为 0 */
	void ResetActionCount() { ActionCount = 0; }

	/** 单次行动开始（由行动 Ability 激活时调用），上报协调器 */
	void NotifyActionStarted(const FGameplayTagContainer& AssetTags);

	/** 单个 Action 结束（由能力 EndAbility 后调用），回到 Selecting 等待下一个 Action */
	void NotifyActionFinished(bool bInterrupted = false, const FGameplayTagContainer& AssetTags = FGameplayTagContainer());

	/** 本回合所有行动完成，上报协调器 */
	void NotifyTurnFinished();

	/** 本回合挂起（蓄力/等待），上报协调器 */
	void NotifyDeferred();

	/** 防守方实时防御（抽象扩展点）：非己方回合全程可主动触发 */
	void TryReactiveDefense(const FReactiveDefenseRequest& Request, FReactiveDefenseResult& Out);

	/** 防守方受击结算钩子（抽象扩展点）：命中结算时刻触发 */
	void NotifyHitResolve(const FHitResolveContext& Context);

protected:
	/** 内部：统一状态迁移入口（集中管理 State 赋值，记录迁移日志） */
	void SetState(ECharacterTurnState NewState);

	/** 所属协调器（弱引用，避免强引用环） */
	UPROPERTY(Transient)
	TWeakObjectPtr<USVCombatTurnCoordinator> Coordinator;

	/** 跨回合待办：蓄力/延迟/持续 N 回合 */
	UPROPERTY(Transient)
	TArray<FPendingTurnAction> PendingActions;

	/** 本回合已发起的行动次数（每回合清零，HandleBeginTurn 时重置） */
	UPROPERTY(Transient)
	int32 ActionCount = 0;

private:
	/** 角色自身回合阶段 */
	UPROPERTY(Transient)
	ECharacterTurnState State = ECharacterTurnState::Idle;

	/** 上一个角色阶段（调试/观测用，SetState 迁移前记录） */
	UPROPERTY(Transient)
	ECharacterTurnState PreviousState = ECharacterTurnState::Idle;

	/** 本回合职责（协调器下发） */
	UPROPERTY(Transient)
	ECombatTurnRole TurnRole = ECombatTurnRole::None;

	/** 内部：进入决策阶段（Selecting/Defending）时的处理（状态机迁移钩子：计算可激活能力并决定是否结束回合） */
	void OnEnterDecisionMaking();

	/** 内部：进入 Acted（结束）状态时的处理（状态机迁移钩子：清空缓存并上报协调器） */
	void OnEnterActed();

	/** 内部：进入 Deferred（中断/挂起）状态时的处理（状态机迁移钩子：上报协调器，不清空缓存） */
	void OnEnterDeferred();

	/** 内部：返回当前 TurnRole 对应的「应该激活的 Tags」配置数组（TurnRole 无效时返回空数组） */
	TArray<FGameplayTag> GetRoleTags() const;

	/** 内部：按 TurnRole 选配置数组，将 AssetTags 中精确命中的配置 tag 去重添加到激活缓存 */
	void AddActiveActionTags(const FGameplayTagContainer& AssetTags);

	/** 内部：从 AssetTags 中精确命中当前职责配置的第一个 Tag（无命中返回空 Tag） */
	FGameplayTag FindRoleTag(const FGameplayTagContainer& AssetTags) const;

	/** 内部：将 ECharacterTurnState 映射为对应的状态机 Tag（State.Turn.*） */
	static FGameplayTag GetStateTag(ECharacterTurnState InState);

	/** 内部：将当前 State 的状态 Tag 挂到角色 ASC 的 LooseTag（移除旧、添加新） */
	void ApplyStateTagToASC();

	/** 内部：获取并缓存角色的 ASC（失效时重新查找，避免每次调用 GetAbilitySystemComponent） */
	UAbilitySystemComponent* GetCachedASC() const;

	/** 攻击方（主动出手）能力 Tags 配置（拷贝自 AbilitySet，运行期只读） */
	UPROPERTY(Transient)
	TArray<FGameplayTag> AttackerTagsConfig;

	/** 防守方（被动响应）能力 Tags 配置（拷贝自 AbilitySet，运行期只读） */
	UPROPERTY(Transient)
	TArray<FGameplayTag> DefenderTagsConfig;

	/** 已激活的行动 Tags 缓存（记录自己作为攻击方/防守方激活过哪些能力，累积不清除） */
	UPROPERTY(Transient)
	TArray<FGameplayTag> ActiveActionTags;

	/** 当前正在 Acting 的能力 Tag（行动开始添加、结束移除；来自 GetRoleTags 精确匹配） */
	UPROPERTY(Transient)
	FGameplayTag CurrentActingTag;

	/** 当前已挂到 ASC 的状态 Tag（用于切换时精确移除上一个，O(1)） */
	UPROPERTY(Transient)
	FGameplayTag CurrentAppliedStateTag;

	/** 缓存的角色 ASC（弱引用，失效时重新查找） */
	UPROPERTY(Transient)
	mutable TWeakObjectPtr<UAbilitySystemComponent> CachedASC;

	/** 攻击方（主动出手）能力 SpecHandle 缓存（Tag → Handle，初始化时归集） */
	UPROPERTY(Transient)
	TMap<FGameplayTag, FGameplayAbilitySpecHandle> AttackerAbilityHandles;

	/** 防守方（被动响应）能力 SpecHandle 缓存（Tag → Handle，初始化时归集） */
	UPROPERTY(Transient)
	TMap<FGameplayTag, FGameplayAbilitySpecHandle> DefenderAbilityHandles;
};
