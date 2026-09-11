// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CatAttributeSetBase.h"
#include "AbilitySystemComponent.h"
#include "CatGradientAttributeSet.generated.h"

/** Gradient 充能变化（供全队共享的分段条 UI 监听） */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCatGradientChangedDelegate, float, NewCharges, float, MaxCharges, float, Progress);

/** 攒满一格 Gradient 充能（GC）时广播一次 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCatGradientChargeGainedDelegate);

/**
 * Gradient 属性集（L4：战斗节奏资源，全队共享）。
 *
 * 挂载约定是硬性的：每队只有一份，挂在队伍共享的 ASC 上（队伍或队长 Actor），
 * 不要挂到单个角色。官方把它做成队伍头像框下方的共享分段条，一队一池。
 *
 * 运行方式：
 *   - 消耗 AP 施放技能累积进度，攒满一段得到 1 个 Gradient Charge（GC）；
 *   - Gradient Attack 按 1GC / 2GC / 3GC 计价，Gradient Counter 也吃这一池；
 *   - 用 Gradient Attack 不会结束回合，相当于一回合出两次手；
 *   - 进攻终极技和保命反击共用同一个池，花在哪边是团队决策。
 *
 * 因此用「充能数 + 当前段进度」建模，而不是单一百分比：
 *   `GradientCharges`    已攒满的 GC 数，可直接消费
 *   `MaxGradientCharges` GC 上限（官方未给出，按最高消耗 3GC 取默认 3，可配）
 *   `GradientProgress`   当前正在攒的那一段的进度 [0, 1)
 */
UCLASS()
class GAMEWORK_API UCatGradientAttributeSet : public UCatAttributeSetBase
{
	GENERATED_BODY()

public:
	UCatGradientAttributeSet();

	/** 已攒满的 Gradient 充能数（GC），可直接消费 */
	UPROPERTY(BlueprintReadOnly, Category = "Cat|Gradient")
	FGameplayAttributeData GradientCharges;
	ATTRIBUTE_ACCESSORS_BASIC(UCatGradientAttributeSet, GradientCharges);

	/** Gradient 充能上限（GC） */
	UPROPERTY(BlueprintReadOnly, Category = "Cat|Gradient")
	FGameplayAttributeData MaxGradientCharges;
	ATTRIBUTE_ACCESSORS_BASIC(UCatGradientAttributeSet, MaxGradientCharges);

	/** 当前正在积攒的那一段的进度 [0, 1) */
	UPROPERTY(BlueprintReadOnly, Category = "Cat|Gradient")
	FGameplayAttributeData GradientProgress;
	ATTRIBUTE_ACCESSORS_BASIC(UCatGradientAttributeSet, GradientProgress);

	/** 充能 / 进度变化广播 */
	UPROPERTY(BlueprintAssignable, Category = "Cat|Gradient")
	FCatGradientChangedDelegate OnGradientChanged;

	/** 攒满一格 GC 时广播（用于播提示音 / 特效） */
	UPROPERTY(BlueprintAssignable, Category = "Cat|Gradient")
	FCatGradientChargeGainedDelegate OnGradientChargeGained;

	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

protected:
	// ~begin UCatAttributeSetBase
	virtual void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const override;
	// ~end UCatAttributeSetBase
};
