// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TimerManager.h"

class UAbilitySystemComponent;

//=====================================================================
// ===== [GAS_MOD_09] START=====
// 新增文件：回合 Timer 管理器（TurnBased Support）
// 整个文件为相对引擎的新增内容，为 GE 的 Duration/Period 提供"回合驱动"能力。
// 包含两个新增类型：
//   - FAbilityTimerContainer：每个 ASC 的回合计时容器
//   - FAbilityTimerManager：回合 Timer 管理器（继承 FTimerManager）
//=====================================================================

/** 每个 ASC 的回合计时容器：记录该 ASC 当前回合数，以及其下所有按回合推进的 Timer。 */
struct FAbilityTimerContainer
{
	/** 当前回合数（由 FAbilityTimerManager::TickTurn 递增） */
	int32 Turn = 0;

	/** 该 ASC 下所有按回合推进的 Timer 句柄 */
	TArray<FTimerHandle> TimerHandles;
};

/**
 * 回合 Timer 管理器：继承实时 FTimerManager，叠加"回合驱动"能力。
 * 使 GE 的 Duration / Period 从"秒"切换为"回合"，由 TickTurn 手动推进。
 * 注意：本管理器不会主动调用父类 FTimerManager::Tick（即不随世界时间走），
 *      回合 Timer 的到期完全由 TickTurn 按回合数触发。
 */
class FAbilityTimerManager : public FTimerManager
{
public:
	FAbilityTimerManager() : FTimerManager(nullptr) {}

	/** 推进某 ASC 的回合（Delta 步进），并触发到期的回合 Timer */
	void TickTurn(UAbilitySystemComponent* ASC, int32 Delta = 1);

	/** 替代 FTimerManager::SetTimer —— 把 Timer 注册进回合容器（Rate 语义为回合数） */
	void SetAbilityTimer(UAbilitySystemComponent* ASC, FTimerHandle& OutHandle, const FTimerDelegate& InDelegate, float InRate, bool bInLoop, float InFirstDelay = -1.f);

	/** 查询某 Timer 剩余回合数 */
	float GetAbilityTimerRemaining(UAbilitySystemComponent* ASC, FTimerHandle& InHandle);

	/** 查询某 ASC 当前回合数 */
	int32 GetAbilityCurrentTurn(UAbilitySystemComponent* ASC) const;

	/** 从回合容器中移除某 Timer（不清理 FTimerManager 内部数据，通常需配合 ClearTimer） */
	void RemoveAbilityTimer(UAbilitySystemComponent* ASC, FTimerHandle& InHandle);

private:
	/** 每个 ASC 独立维护自己的回合计时容器 */
	TMap<TWeakObjectPtr<UAbilitySystemComponent>, FAbilityTimerContainer> AbilityTimerContainers;

	/** 获取（必要时创建）某 ASC 的回合计时容器 */
	FAbilityTimerContainer& GetAbilityTimerContainer(UAbilitySystemComponent* ASC);
};

// ===== [GAS_MOD_09] END =====
//=====================================================================
