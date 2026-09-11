// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CatAttributeSetBase.h"
#include "AbilitySystemComponent.h"
#include "CatPostureAttributeSet.generated.h"

/** 姿态条变化（供敌人 Break 条 UI 监听） */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCatBreakChangedDelegate, float, NewBreak, float, MaxBreak);

/** Break 达到满值、可被引爆 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCatBreakReadyDelegate);

/**
 * 姿态属性集（L4：战斗节奏资源，**敌人专用**）。
 *
 * Break 是本作独立于 HP 的「姿态 / 稳定性」条，位于敌人血条**下方**（黄色）。
 * 官方机制（Game8《How to Break Enemies》/《Combat Guide》），本设计与之一一对应：
 *   1. **积累**：敌人**每次受到伤害**都会填充 Break 条（不需要特定条件）。
 *   2. **引爆**：打满还不够 —— **必须用带「Can Break」标记的技能命中**，才会真正进入 Broken 状态。
 *   3. **效果**：Broken → 施加 **Stun（眩晕）**；部分敌人还会暴露 **Weak Spot（弱点）**。
 *
 * 因此本集只负责第 1 步（把 Break 顶到 MaxBreak 并置 bBreakReady / 广播 OnBreakReady），
 * 第 2、3 步由带 Can Break 标记的引爆技能写入 StunTurns —— 「积攒 / 引爆」两段式与官方一致。
 *
 * 与护盾是**两套完全独立的系统**：护盾用瞄准射击破、Break 用伤害积累（见 UCatShieldAttributeSet）。
 *
 * 注：官方未给出 Break 条的具体数值，也未说明玩家是否会被 Break（本文档按「仅敌人」处理）。
 */
UCLASS()
class GAMEWORK_API UCatPostureAttributeSet : public UCatAttributeSetBase
{
	GENERATED_BODY()

public:
	UCatPostureAttributeSet();

	/** 已积累的 Break 值 [0, MaxBreak] */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Break, Category = "Cat|Posture")
	FGameplayAttributeData Break;
	ATTRIBUTE_ACCESSORS_BASIC(UCatPostureAttributeSet, Break);

	/** Break 上限（满值） */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxBreak, Category = "Cat|Posture")
	FGameplayAttributeData MaxBreak;
	ATTRIBUTE_ACCESSORS_BASIC(UCatPostureAttributeSet, MaxBreak);

	/** 每回合自然回复量（挂在 TimeAxis.Round.Start 的 GE 读取） */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_BreakRegen, Category = "Cat|Posture")
	FGameplayAttributeData BreakRegen;
	ATTRIBUTE_ACCESSORS_BASIC(UCatPostureAttributeSet, BreakRegen);

	/** 引爆后眩晕的剩余回合数 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_StunTurns, Category = "Cat|Posture")
	FGameplayAttributeData StunTurns;
	ATTRIBUTE_ACCESSORS_BASIC(UCatPostureAttributeSet, StunTurns);

	/** 是否已达可引爆状态（Break >= MaxBreak）。由 PostAttributeChange 派生，服务器与客户端一致，无需复制 */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Cat|Posture")
	bool bBreakReady = false;

	/** 姿态条变化广播 */
	UPROPERTY(BlueprintAssignable, Category = "Cat|Posture")
	FCatBreakChangedDelegate OnBreakChanged;

	/** 达到可引爆状态时广播一次 */
	UPROPERTY(BlueprintAssignable, Category = "Cat|Posture")
	FCatBreakReadyDelegate OnBreakReady;

	// ------------------------------------------------------------------
	// 回调
	// ------------------------------------------------------------------

	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// ~begin UCatAttributeSetBase
	virtual void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const override;
	// ~end UCatAttributeSetBase

	UFUNCTION()
	void OnRep_Break(const FGameplayAttributeData& OldBreak);

	UFUNCTION()
	void OnRep_MaxBreak(const FGameplayAttributeData& OldMaxBreak);

	UFUNCTION()
	void OnRep_BreakRegen(const FGameplayAttributeData& OldBreakRegen);

	UFUNCTION()
	void OnRep_StunTurns(const FGameplayAttributeData& OldStunTurns);
};
