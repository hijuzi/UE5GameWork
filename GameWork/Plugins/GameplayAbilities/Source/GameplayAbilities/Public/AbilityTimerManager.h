// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TimerManager.h"
#include "GameplayTagContainer.h"

class UAbilitySystemComponent;

//=====================================================================
// ===== [GAS_MOD_09] START=====
// 新增文件：时间轴 Timer 管理器（TurnBased Support + 多时间轴）
//
// 相对引擎为全新文件；当前形态 = 按时机（GameplayTag）分组的"多条时间轴"。
// 多时间轴改造已并入本编号 GAS_MOD_09。
//
// 历史对照：
//   改造前(单刻度版)：
//     struct FAbilityTimerContainer { int32 Turn = 0; TArray<FTimerHandle> TimerHandles; };
//     void TickTurn(ASC, Delta);
//     void SetAbilityTimer(ASC, FTimerHandle&, Delegate, Rate, bLoop, FirstDelay);
//     float GetAbilityTimerRemaining(ASC, FTimerHandle&);
//     int32 GetAbilityCurrentTurn(ASC) const;
//     void RemoveAbilityTimer(ASC, FTimerHandle&);
//
//   改造后(当前, 多时间轴版)：
//     - 容器按 FGameplayTag（时机）分组成多条轴，每条轴独立计数、独立挂载 Timer；
//     - 推进入口 TickTimeline(ASC, Timing, Delta)：命中所有"被匹配的轴"（含父级聚合，
//       例如 InTiming = TimeAxis.Action.Attack 会同时命中 Action.Attack 轴与父级 Action 轴）；
//     - 注册/查询/清理接口全部带上 Timing，明确归属；
//     - 移除 TickTurn / GetAbilityCurrentTurn，分别由 TickTimeline / GetTimelineCounter 取代。
//
// 包含类型：
//   - FAbilityTimerAxis：单条时间轴（某个时机）的运行状态
//   - FAbilityTimerContainer：每个 ASC 的计时容器（按时机分轴）
//   - FAbilityTimerManager：时间轴 Timer 管理器（继承 FTimerManager）
//=====================================================================

/** 某条时间轴（某个"时机"）的运行状态 */
struct FAbilityTimerAxis
{
	/** 该时机已推进到的刻度（回合数 / 出手次数，语义由调用方决定） */
	int32 Counter = 0;

	/** 挂在该时机上的所有 Timer 句柄 */
	TArray<FTimerHandle> TimerHandles;
};

/** 每个 ASC 的计时容器：按时机 Tag 分组，各轴独立推进 */
struct FAbilityTimerContainer
{
	/** key = 时机（TimeAxis.*）；value = 该时机的运行状态 */
	TMap<FGameplayTag, FAbilityTimerAxis> Axes;
};

/**
 * 时间轴 Timer 管理器：继承实时 FTimerManager，叠加"按时机驱动"能力。
 * 使 GE 的 Duration / Period 从"秒"切换为"时机刻度"，由 TickTimeline 手动推进。
 * 注意：本管理器不会主动调用父类 FTimerManager::Tick（即不随世界时间走），
 *      各轴 Timer 的到期完全由 TickTimeline 按刻度触发。
 */
class FAbilityTimerManager : public FTimerManager
{
public:
	FAbilityTimerManager() : FTimerManager(nullptr) {}

	/** 推进某 ASC 的指定时机（Delta 步进），命中所有匹配轴（含父级聚合） */
	void TickTimeline(UAbilitySystemComponent* ASC, FGameplayTag InTiming, int32 Delta = 1);

	/** 注册 Timer 到指定时机（Rate 语义为该时机刻度数） */
	void SetAbilityTimer(UAbilitySystemComponent* ASC, FGameplayTag InTiming, FTimerHandle& OutHandle, const FTimerDelegate& InDelegate, float InRate, bool bInLoop, float InFirstDelay = -1.f);

	/** 查询某 Timer 在指定时机上的剩余刻度 */
	float GetAbilityTimerRemaining(UAbilitySystemComponent* ASC, FGameplayTag InTiming, FTimerHandle& InHandle);

	/** 查询某 ASC 指定时机当前的刻度 */
	int32 GetTimelineCounter(UAbilitySystemComponent* ASC, FGameplayTag InTiming) const;

	/** 从指定时机移除某 Timer（不清理 FTimerManager 内部数据，通常需配合 ClearTimer） */
	void RemoveAbilityTimer(UAbilitySystemComponent* ASC, FGameplayTag InTiming, FTimerHandle& InHandle);

private:
	/** 每个 ASC 独立维护自己的计时容器 */
	TMap<TWeakObjectPtr<UAbilitySystemComponent>, FAbilityTimerContainer> AbilityTimerContainers;

	/** 获取（必要时创建）某 ASC 的计时容器 */
	FAbilityTimerContainer& GetAbilityTimerContainer(UAbilitySystemComponent* ASC);

	/** 获取（必要时创建）某 ASC 指定时机的时间轴 */
	FAbilityTimerAxis& GetAbilityTimerAxis(UAbilitySystemComponent* ASC, FGameplayTag InTiming);

	/** 在全部轴中按句柄查找并移除（用于注册前清理旧句柄，或时机未知时的兜底移除） */
	void RemoveAbilityTimerByHandle(UAbilitySystemComponent* ASC, FTimerHandle& InHandle);

	/**
	 * 整条轴同步平移：Counter 与轴上所有 Timer 的 ExpireTime 同减 InRebase。
	 * 判定只看 (ExpireTime - Counter) 的差值，平移后触发行为完全不变；
	 * 用于在 Axis.Counter 逼近 int32 上限前把刻度拉回小值，规避有符号溢出。
	 */
	void RebaseAxis(FAbilityTimerAxis& InAxis, int32 InRebase);
};

// ===== [GAS_MOD_09] END =====
//=====================================================================
