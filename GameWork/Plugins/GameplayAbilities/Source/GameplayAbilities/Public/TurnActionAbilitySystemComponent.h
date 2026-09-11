// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ActiveGameplayEffectHandle.h"
#include "AbilitySystemComponent.h"
#include "TurnActionAbilitySystemComponent.generated.h"

//=====================================================================
// ===== [GAS_MOD_16] START=====
// 新增文件：回合制专用 ASC 子类（TurnAction）。
//
// 目的：不改动引擎源码，为「回合制 GE」补齐三类能力：
//   ① 蓝图可绑事件 —— 时间轴推进 / GE 挂载 / GE 卸载。
//      引擎既有的 GE 生命周期委托（OnActiveGameplayEffectAddedDelegateToSelf、
//      OnAnyGameplayEffectRemovedDelegate 等）均为原生 DECLARE_MULTICAST_DELEGATE
//      且不是 UPROPERTY，蓝图无法直接绑定；引擎也没有「时间轴推进」这个事件。
//   ② 查询 —— 当前刻度（第几回合 / 第几次出手）、GE 所属时机、GE 剩余刻数。
//   ③ 复制层 —— 把「各时间轴刻度」复制给客户端。
//      权威端刻度由 FAbilityTimerManager 维护，默认不随 ASC 复制，故客户端需要这份副本
//      才能得到轮次信息（并在客户端驱动 OnTimelineTicked）。
//
// ⚠ 推进入口（方案 A：零引擎改动）
//   本类的 TickTimeline(Timing, Delta) 是**唯一**推进入口：内部先调
//   FAbilityTimerManager::TickTimeline，再同步复制副本并逐轴广播 OnTimelineTicked。
//   UAbilitySystemBlueprintLibrary::TickTimeline 已加 meta=(DeprecatedFunction) 正式弃用：
//   它不经过本类 → 不会同步复制副本、不会广播 OnTimelineTicked，客户端轮次会停在旧值。
//=====================================================================

class UTurnActionAbilitySystemComponent;

/** 单条时间轴的刻度副本（网络复制用） */
USTRUCT(BlueprintType)
struct GAMEPLAYABILITIES_API FTurnTimelineCounter
{
	GENERATED_BODY()

	/** 时间轴（时机）Tag */
	UPROPERTY(BlueprintReadOnly, Category = "TurnBased")
	FGameplayTag Timing;

	/** 该轴的刻度（回合数 / 出手次数，语义由玩法决定） */
	UPROPERTY(BlueprintReadOnly, Category = "TurnBased")
	int32 Counter = 0;
};

/** 回合制 GE 生命周期事件负载（挂载 / 卸载共用） */
USTRUCT(BlueprintType)
struct GAMEPLAYABILITIES_API FTurnBasedEffectEvent
{
	GENERATED_BODY()

	/** 该 ActiveGE 的句柄 */
	UPROPERTY(BlueprintReadOnly, Category = "TurnBased")
	FActiveGameplayEffectHandle Handle;

	/** GE 资产（用于区分是哪个效果） */
	UPROPERTY(BlueprintReadOnly, Category = "TurnBased")
	TSubclassOf<UGameplayEffect> GameplayEffectClass;

	/** 该 GE 挂在哪条时间轴（未配置时回落 TimeAxis.Round.End） */
	UPROPERTY(BlueprintReadOnly, Category = "TurnBased")
	FGameplayTag Timing;

	/** 事件发生时刻，该时间轴推进到的刻度 */
	UPROPERTY(BlueprintReadOnly, Category = "TurnBased")
	int32 TimelineCounter = 0;

	/** 该 GE 挂载时的刻度；卸载时 TimelineCounter - StartCounter 即存活刻数 */
	UPROPERTY(BlueprintReadOnly, Category = "TurnBased")
	int32 StartCounter = 0;

	/** 卸载事件有效：true = 被主动移除，false = 自然到期（客户端不可靠，见 .cpp 注释） */
	UPROPERTY(BlueprintReadOnly, Category = "TurnBased")
	bool bPrematureRemoval = false;

	/** 挂载事件有效：true = 由「抑制恢复」重新激活（同一 GE 会再次触发），false = 首次挂载 */
	UPROPERTY(BlueprintReadOnly, Category = "TurnBased")
	bool bIsReactivation = false;
};

/** 某条时间轴被推进到第 NewCounter 刻 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FTurnTimelineTicked,
	UTurnActionAbilitySystemComponent*, AbilitySystemComponent, FGameplayTag, Timing, int32, NewCounter);

/** 回合制 GE 挂载 / 卸载 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTurnBasedEffectEventSignature,
	UTurnActionAbilitySystemComponent*, AbilitySystemComponent, const FTurnBasedEffectEvent&, Event);

/**
 * 回合制专用 AbilitySystemComponent。
 *
 * 构造函数中已将 bTurnBased 置为 true —— 「类型即语义」：用本类即表示该 ASC 走回合制。
 * 框架层仍可只按 UAbilitySystemComponent* 使用（向上转型），需要观测能力时再 Cast 到本类。
 */
UCLASS(ClassGroup = (Abilities), meta = (BlueprintSpawnableComponent))
class GAMEPLAYABILITIES_API UTurnActionAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UTurnActionAbilitySystemComponent();

	// ---------------- 获取组件（蓝图可用） ----------------

	/**
	 * 从 Actor 上取回合制 ASC。
	 * 内部等价于 `Cast<UTurnActionAbilitySystemComponent>(GetAbilitySystemComponent(Actor))`：
	 * 先按 IAbilitySystemInterface / 组件查找取到 ASC，再 Cast 到本类；不是本类则返回 nullptr。
	 *
	 * `DefaultToSelf` 让蓝图在 Actor 上下文里调用时自动填 Self。
	 */
	UFUNCTION(BlueprintPure, Category = "TurnBased", meta = (DefaultToSelf = "Actor"))
	static UTurnActionAbilitySystemComponent* GetTurnActionAbilitySystemComponent(AActor* Actor);

	// ---------------- 推进入口（唯一） ----------------

	/**
	 * 推进本 ASC 的某条时机（Delta 步进），命中所有匹配轴（含父级聚合）。
	 * 内部：FAbilityTimerManager::TickTimeline → 同步复制副本 → 逐轴广播 OnTimelineTicked。
	 */
	UFUNCTION(BlueprintCallable, Category = "TurnBased")
	void TickTimeline(FGameplayTag Timing, int32 Delta = 1);

	// ---------------- 查询 ----------------

	/** 某条时间轴当前刻度（第几回合 / 第几次出手）。权威端实时；客户端读复制副本。 */
	UFUNCTION(BlueprintPure, Category = "TurnBased")
	int32 GetTimelineRound(FGameplayTag Timing) const;

	/** 某 GE 挂在哪条时间轴（未配置时回落 TimeAxis.Round.End） */
	UFUNCTION(BlueprintPure, Category = "TurnBased")
	FGameplayTag GetGameplayEffectTiming(FActiveGameplayEffectHandle Handle) const;

	/** 某回合制 GE 还剩几刻（-1 = 无期限 / 无效句柄 / 客户端信息不足） */
	UFUNCTION(BlueprintPure, Category = "TurnBased")
	int32 GetGameplayEffectRemainingTicks(FActiveGameplayEffectHandle Handle) const;

	/** 某回合制 GE 已存活几刻（-1 = 未知） */
	UFUNCTION(BlueprintPure, Category = "TurnBased")
	int32 GetGameplayEffectElapsedTicks(FActiveGameplayEffectHandle Handle) const;

	// ---------------- 事件（蓝图可绑） ----------------

	/** 某条时间轴被推进（每轴每步一次；客户端由复制副本 OnRep 驱动） */
	UPROPERTY(BlueprintAssignable, Category = "TurnBased")
	FTurnTimelineTicked OnTimelineTicked;

	/** 回合制 GE 挂载（从「抑制恢复」重新激活时会再次触发，bIsReactivation = true） */
	UPROPERTY(BlueprintAssignable, Category = "TurnBased")
	FTurnBasedEffectEventSignature OnTurnBasedEffectAdded;

	/** 回合制 GE 卸载（bPrematureRemoval 区分自然到期 / 被主动移除） */
	UPROPERTY(BlueprintAssignable, Category = "TurnBased")
	FTurnBasedEffectEventSignature OnTurnBasedEffectRemoved;

protected:
	virtual void OnRegister() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** 各时间轴刻度副本（权威端推进时更新；客户端据此得到轮次并驱动 OnTimelineTicked） */
	UPROPERTY(ReplicatedUsing = OnRep_TimelineCounters)
	TArray<FTurnTimelineCounter> TimelineCounters;

	UFUNCTION()
	void OnRep_TimelineCounters();

private:
	/** 绑定到自身 OnActiveGameplayEffectAddedDelegateToSelf 的回调 */
	void HandleActiveEffectAdded(UAbilitySystemComponent* SourceASC, const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle Handle);

	/** 绑定到自身 OnAnyGameplayEffectRemovedDelegate 的回调 */
	void HandleActiveEffectRemoved(const FActiveGameplayEffect& Effect);

	/** 在复制副本中查找 / 新建某轴的条目 */
	FTurnTimelineCounter& FindOrAddTimelineCounter(FGameplayTag Timing);

	/** 已登记「挂载」的 GE（用于把「抑制恢复」识别为重激活） */
	TSet<FActiveGameplayEffectHandle> KnownEffectHandles;

	/** GE → 挂载时刻度（卸载时用于算存活刻数） */
	TMap<FActiveGameplayEffectHandle, int32> EffectStartCounters;

	/** 客户端镜像：上一次收到的刻度（用于在 OnRep 中 diff 出「哪些轴被推进了」） */
	TMap<FGameplayTag, int32> LastReplicatedCounters;

	/** 是否已收到过首份刻度副本（首次只登记镜像，不广播推进事件） */
	bool bReceivedInitialCounters = false;

	/** 是否已绑定原生委托（OnRegister 可能被多次调用） */
	bool bBoundDelegates = false;
};

// ===== [GAS_MOD_16] END =====
//=====================================================================
