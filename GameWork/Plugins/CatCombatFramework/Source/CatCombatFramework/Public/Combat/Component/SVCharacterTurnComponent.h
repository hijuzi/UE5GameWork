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
class UGameplayAbility;

/**
 * 角色级回合组件（决策层，自驱动状态机）。
 * 挂载在战斗角色上，持有角色自身回合阶段与跨回合行动队列。
 *
 * 不决定"下一个轮到谁"（那是全局协调器的职责），只决定"我此刻做什么、何时上报完成"。
 * 由角色类的 ISVCombatCoreInterface::OnBeginTurn_Implementation 转发调用 HandleBeginTurn。
 *
 * 战斗能力采用「注册制」：授予方在给 ASC 授予能力时通过 RegisterCombatAbility 显式注册
 * （携带 FSVTurnActionData 限次配置），组件据此维护 Tag → Handle 映射与激活限次计数
 * （PerTurn / WholeBattle / Unlimited），供决策层查询剩余可行动能力。
 */
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class CATCOMBATFRAMEWORK_API USVCharacterTurnComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USVCharacterTurnComponent();

	/**
	 * 每回合最大行动槽位数（0=不限，默认）。
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Turn", meta = (ClampMin = "0", UIMin = "0"))
	int32 MaxActionCountPerTurn = 0;

	/** 获取指定 Actor 上的回合组件（静态，供外部快速查询；Actor 无效或无组件时返回 nullptr） */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat|Turn")
	static USVCharacterTurnComponent* GetSVCharacterTurnComponent(const AActor* Actor);

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

	/** 注册单个战斗能力：根据 TurnActionData 将能力的主标识 Tag 与 SpecHandle 加入对应职责映射（由能力授予时自行调用） */
	void RegisterCombatAbility(const FSVTurnActionData& TurnActionData, const FGameplayTag& Tag, const FGameplayAbilitySpecHandle& Handle);

	/** 注销单个战斗能力：根据 TurnActionData 从对应职责映射移除该 Handle 的所有 Tag 条目（能力移除时调用） */
	void UnregisterCombatAbility(const FSVTurnActionData& TurnActionData, const FGameplayAbilitySpecHandle& Handle);

	/** 获取攻击方能力 Tags（从映射 keys 生成，只读） */
	TArray<FGameplayTag> GetAttackerTags() const;

	/** 获取防守方能力 Tags（从映射 keys 生成，只读） */
	TArray<FGameplayTag> GetDefenderTags() const;

	/** 通过 Tag 获取对应的能力实例（从 Handle 映射经 ASC 解析；未找到返回 nullptr） */
	UGameplayAbility* GetAbilityInstanceByTag(const FGameplayTag& Tag, ECombatTurnRole Role) const;

	/** 获取攻击方能力 TurnActionData 缓存（Tag → 配置，只读，供调试分类/查询） */
	const TMap<FGameplayTag, FSVTurnActionData>& GetAttackerAbilityData() const { return AttackerAbilityData; }

	/** 获取防守方能力 TurnActionData 缓存（Tag → 配置，只读，供调试分类/查询） */
	const TMap<FGameplayTag, FSVTurnActionData>& GetDefenderAbilityData() const { return DefenderAbilityData; }

	/** 获取合并后的能力激活次数映射（Tag → 已激活次数，by value，供调试/查询） */
	TMap<FGameplayTag, int32> GetAllActivationCounts() const;

	/** 获取「回合内限次（Limited + PerTurn）」激活次数（只读，供调试显示） */
	const TMap<FGameplayTag, int32>& GetPerTurnActivationCounts() const { return PerTurnActivationCounts; }

	/** 获取「整场限次（Limited + WholeBattle）」激活次数（只读，供调试显示） */
	const TMap<FGameplayTag, int32>& GetBattleActivationCounts() const { return BattleActivationCounts; }

	/** 获取「无限次数（Unlimited）」激活次数（只读，供调试显示） */
	const TMap<FGameplayTag, int32>& GetUnlimitedActivationCounts() const { return UnlimitedActivationCounts; }

	/** 获取当前正在 Acting 的能力 Tag（无则为空 Tag，只读，供调试/观测） */
	FGameplayTag GetCurrentActingTag() const { return CurrentActingTag; }

	/** 获取当前正在 Acting 的能力对应的 GA Class（用于调试显示，无则返回 nullptr） */
	UClass* GetCurrentActingAbilityClass() const;

	/** 判断指定 Tag 对应的能力当前能否激活（按 Role 选 Handle Map，调用 ASC 原生 CanActivateAbility） */
	bool CanActivateAbilityByTag(const FGameplayTag& Tag, ECombatTurnRole Role) const;

	/** AI 激活入口：按 Tag 从当前职责 Handle 缓存取 SpecHandle 并激活（复用 ASC 原生 TryActivateAbility），返回是否成功触发 */
	bool TryActivateActionByTag(const FGameplayTag& Tag, ECombatTurnRole Role);

	/** 计算本回合 TurnAction 列表剩余可执行的行动 Tag（还有哪些待执行；为空表示全部执行完、结束当前回合）。
	 *  规则 1：标记「bEndTurnWhenLimitReached」的 Limited 技能次数用尽即返回空（立即结束回合），其余技能用尽仅移除自身；
	 *  规则 2：非忽略限制能力才计入——默认仅收集本回合尚未用过（激活次数 0）的技能；
	 *          bAllowRepeats=true 时，已用过但仍可用（Limited 剩余 > 0 / Unlimited）的技能也可重复进入候选。
	 *  每回合行动次数上限由 MaxActionCountPerTurn 独立控制 */
	FGameplayTagContainer GetRemainingActionTags(bool bAllowRepeats = false) const;

	/** 获取指定 Tag 能力已激活次数（只读；无记录返回 0；按 Role 查配置归类） */
	int32 GetActivationCount(const FGameplayTag& Tag, ECombatTurnRole Role) const;

	/** 指定 Tag 能力激活次数 +1（按 Role 查配置归类：PerTurn/WholeBattle/Unlimited） */
	void IncrementActivationCount(const FGameplayTag& Tag, ECombatTurnRole Role);

	/** 重置指定 Tag 能力激活次数为 0（从所有分类计数中移除） */
	void ResetActivationCount(const FGameplayTag& Tag);

	/** 重置所有能力激活次数（回合开始/战斗重开时调用） */
	void ResetAllActivationCounts();

	/** 仅重置「单个回合内（PerTurn）」能力激活次数（回合切换时调用；WholeBattle 计数保留） */
	void ResetPerTurnActivationCounts();

	/** 获取指定 Tag 能力剩余可激活次数（仅当该能力为「限制次数」类型时返回 true 并输出剩余；否则返回 false） */
	bool GetRemainingActivationCount(const FGameplayTag& Tag, ECombatTurnRole Role, int32& OutRemainingCount) const;

	/** 获取本回合已发起的行动次数（每回合清零，只读，供调试/查询） */
	int32 GetActionCount() const { return ActionCount; }

	/** 行动次数 +1（封装递增逻辑） */
	void IncrementActionCount() { ++ActionCount; }

	/** 重置行动次数为 0 */
	void ResetActionCount() { ActionCount = 0; }

	/** 单次行动开始（由行动 Ability 激活时调用），上报协调器 */
	void NotifyActionStarted(const FGameplayTagContainer& AssetTags);

	/** 单个 Action 结束（由能力 EndAbility 后调用），回到 Selecting 等待下一个 Action */
	void NotifyActionFinished(const FGameplayTagContainer& AssetTags = FGameplayTagContainer());

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

	/** 内部：所属协调器是否已进入「行动收尾（Finishing）」阶段（协调器缺失时返回 false） */
	bool IsCoordinatorFinishing() const;

	/** 内部：调度「结束当前回合」——优先延迟到下一帧执行（避免在 SetState 状态迁移过程中再次触发状态迁移的重入），World 无效时兜底直接上报 */
	void ScheduleTurnFinished();

	/** 内部：是否启用「行动槽位收敛」（攻击方职责 + MaxActionCountPerTurn > 0） */
	bool ShouldEnforceActionSlotLimit() const;

	/** 内部：进入决策阶段（Selecting/Defending）时的处理（状态机迁移钩子：计算可激活能力并决定是否结束回合） */
	void OnEnterDecisionMaking();

	/** 内部：进入 Acted（结束）状态时的处理（状态机迁移钩子：清空缓存并上报协调器） */
	void OnEnterActed();

	/** 内部：进入 Deferred（中断/挂起）状态时的处理（状态机迁移钩子：上报协调器，不清空缓存） */
	void OnEnterDeferred();

	/** 内部：返回当前 TurnRole 对应的「应该激活的 Tags」配置数组（TurnRole 无效时返回空数组） */
	TArray<FGameplayTag> GetRoleTags() const;

	/** 内部：从 AssetTags 中精确命中当前职责配置的第一个 Tag（无命中返回空 Tag） */
	FGameplayTag FindRoleTag(const FGameplayTagContainer& AssetTags) const;

	/** 内部：将 ECharacterTurnState 映射为对应的状态机 Tag（State.Turn.*） */
	static FGameplayTag GetStateTag(ECharacterTurnState InState);

	/** 内部：将当前 State 的状态 Tag 挂到角色 ASC 的 LooseTag（移除旧、添加新） */
	void ApplyStateTagToASC();

	/** 内部：获取并缓存角色的 ASC（失效时重新查找，避免每次调用 GetAbilitySystemComponent） */
	UAbilitySystemComponent* GetCachedASC() const;

	/** 内部：按 Role 从对应 DataMap 查 Tag 的 TurnActionData（未注册返回 nullptr） */
	const FSVTurnActionData* FindAbilityData(const FGameplayTag& Tag, ECombatTurnRole Role) const;

	/** 内部：返回 Tag 应计入的计数 map（按 LimitType/LimitScope 归类；IgnoreLimit/未注册返回 nullptr 表示不记录） */
	const TMap<FGameplayTag, int32>* GetCountMapForTag(const FGameplayTag& Tag, ECombatTurnRole Role) const;

	/** 内部：非 const 版（IgnoreLimit/未注册返回 nullptr 表示不记录） */
	TMap<FGameplayTag, int32>* GetCountMapForTag(const FGameplayTag& Tag, ECombatTurnRole Role);

	/** Limited + PerTurn（单个回合内限次）：能力激活次数，回合切换时清零 */
	UPROPERTY(Transient)
	TMap<FGameplayTag, int32> PerTurnActivationCounts;

	/** Limited + WholeBattle（整场战斗限次）：能力激活次数，加入战斗/重开时清零 */
	UPROPERTY(Transient)
	TMap<FGameplayTag, int32> BattleActivationCounts;

	/** Unlimited（无限次数）：能力激活次数，仅观测用途（IgnoreLimit 忽略限制，不记录） */
	UPROPERTY(Transient)
	TMap<FGameplayTag, int32> UnlimitedActivationCounts;

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

	/** 攻击方能力 TurnActionData 缓存（Tag → TurnActionData，注册时缓存，供限制次数判定） */
	UPROPERTY(Transient)
	TMap<FGameplayTag, FSVTurnActionData> AttackerAbilityData;

	/** 防守方能力 TurnActionData 缓存（Tag → TurnActionData，注册时缓存，供限制次数判定） */
	UPROPERTY(Transient)
	TMap<FGameplayTag, FSVTurnActionData> DefenderAbilityData;
};
