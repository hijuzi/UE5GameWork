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
 * 姿态属性集（L4：战斗节奏资源，敌人专用）。
 *
 * Break 是敌人血条下方的独立条，不在 HP 里，也和护盾不是一套东西。
 * 这个系统分两段：
 *   1. 积累：敌人每次受到伤害都会填充 Break 条，不需要特定条件；
 *   2. 引爆：打满还不够，必须用带「Can Break」标记的技能命中，才真正进入 Broken 状态，
 *      施加 Stun，部分敌人还会暴露 Weak Spot。
 *
 * 本集只负责第一段：把 `Break` 顶到 `MaxBreak` 时置 `bBreakReady` 并广播 `OnBreakReady`。
 * 第二段的引爆由带标记的技能写 `StunTurns`。因为 Broken 的效果是 Stun，行动相关的处理
 * 可以直接复用项目已有的 `State.Stunned` 状态标签。
 *
 * 官方没有给出 Break 条的具体数值，`MaxBreak = 100` 是本项目自定的基准。
 */
UCLASS()
class GAMEWORK_API UCatPostureAttributeSet : public UCatAttributeSetBase
{
	GENERATED_BODY()

public:
	UCatPostureAttributeSet();

	/** 已积累的 Break 值 [0, MaxBreak] */
	UPROPERTY(BlueprintReadOnly, Category = "Cat|Posture")
	FGameplayAttributeData Break;
	ATTRIBUTE_ACCESSORS_BASIC(UCatPostureAttributeSet, Break);

	/** Break 上限（满值） */
	UPROPERTY(BlueprintReadOnly, Category = "Cat|Posture")
	FGameplayAttributeData MaxBreak;
	ATTRIBUTE_ACCESSORS_BASIC(UCatPostureAttributeSet, MaxBreak);

	/** 每回合自然回复量（挂在 TimeAxis.Round.Start 的 GE 读取） */
	UPROPERTY(BlueprintReadOnly, Category = "Cat|Posture")
	FGameplayAttributeData BreakRegen;
	ATTRIBUTE_ACCESSORS_BASIC(UCatPostureAttributeSet, BreakRegen);

	/** 引爆后眩晕的剩余回合数 */
	UPROPERTY(BlueprintReadOnly, Category = "Cat|Posture")
	FGameplayAttributeData StunTurns;
	ATTRIBUTE_ACCESSORS_BASIC(UCatPostureAttributeSet, StunTurns);

	/** 是否已达可引爆状态（Break >= MaxBreak）。由 PostAttributeChange 派生 */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Cat|Posture")
	bool bBreakReady = false;

	/** 姿态条变化广播 */
	UPROPERTY(BlueprintAssignable, Category = "Cat|Posture")
	FCatBreakChangedDelegate OnBreakChanged;

	/** 达到可引爆状态时广播一次 */
	UPROPERTY(BlueprintAssignable, Category = "Cat|Posture")
	FCatBreakReadyDelegate OnBreakReady;

	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

protected:
	// ~begin UCatAttributeSetBase
	virtual void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const override;
	// ~end UCatAttributeSetBase
};
