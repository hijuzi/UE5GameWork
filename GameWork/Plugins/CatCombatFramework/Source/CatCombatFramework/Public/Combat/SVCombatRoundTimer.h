// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Containers/Ticker.h"
#include "SVCombatRoundTimer.generated.h"

class USVCombatManagerSubsystem;

/** 回合计时器超时事件 */
DECLARE_MULTICAST_DELEGATE(FOnCombatRoundTimeExpired);

/**
 * 回合计时器（执行层，非决策层）。
 * 只负责回合计时计量与超时通知，超时后广播 OnRoundTimeExpired，
 * 由 USVCombatManagerSubsystem 决定如何处理（如 StopCurrentTask）。
 * Tick 生命周期跟随战斗开始/结束，而非 Subsystem 生命周期。
 */
UCLASS()
class CATCOMBATFRAMEWORK_API USVCombatRoundTimer : public UObject
{
	GENERATED_BODY()

public:
	/** 初始化：记录持有者（不注册 Ticker） */
	void Initialize(USVCombatManagerSubsystem* InOwner);

	/** 反初始化：移除 Ticker */
	void Shutdown();

	/** 开始回合计时：完整重置并注册 Ticker */
	void StartRoundTiming();

	/** 停止回合计时：移除 Ticker */
	void StopRoundTiming();

	/** 重置回合计时（换队）：仅清空累计时间与超时标记，保留暂停态 */
	void ResetRoundTime();

	/** 设置暂停状态 */
	void SetPaused(bool bInPaused);

	/** 获取当前回合进度，返回 [0, 1] */
	float GetProgress() const;

	/** 回合计时是否已超时 */
	bool IsExpired() const { return bTimeExpired; }

	/** 获取当前回合累计时间（秒，调试/观测用） */
	float GetCurrentRoundTime() const { return CurrentRoundTime; }

	/** 回合计时是否暂停（调试/观测用） */
	bool IsPaused() const { return bPaused; }

	/** 提供 World 上下文（Outer 为 CombatManagerSubsystem） */
	virtual UWorld* GetWorld() const override;

	/** 回合计时超时事件 */
	FOnCombatRoundTimeExpired OnRoundTimeExpired;

private:
	/** 注册核心 Ticker */
	void RegisterTick();

	/** 移除核心 Ticker */
	void RemoveTick();

	/** Tick 回调，返回 true 继续 tick */
	bool Tick(float DeltaTime);

	/** 检查是否超时，超时则标记并广播 */
	void CheckTimeFinished();

	/** 持有者（弱引用，避免强引用环） */
	UPROPERTY(Transient)
	TWeakObjectPtr<USVCombatManagerSubsystem> OwnerSubsystem;

	/** Tick 句柄，用于 StopRoundTiming/Shutdown 时移除 */
	FTSTicker::FDelegateHandle TickHandle;

	/** 当前回合累计时间（秒） */
	float CurrentRoundTime = 0.0f;

	/** 是否暂停回合计时 */
	bool bPaused = false;

	/** 回合计时是否已超时失效 */
	bool bTimeExpired = false;

	/** 单回合最大时长（秒，缓存自 USVCombatSettings，避免每帧查询 CDO） */
	float MaxRoundTime = 0.0f;
};
