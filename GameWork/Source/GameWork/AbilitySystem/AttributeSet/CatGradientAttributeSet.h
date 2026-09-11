// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CatAttributeSetBase.h"
#include "AbilitySystemComponent.h"
#include "CatGradientAttributeSet.generated.h"

/** Gradient 充能变化（供全队共享的 Gradient 分段条 UI 监听） */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCatGradientChangedDelegate, float, NewCharges, float, MaxCharges, float, Progress);

/** 攒满一格 Gradient 充能（GC）时广播一次 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCatGradientChargeGainedDelegate);

/**
 * Gradient 属性集（L4：战斗节奏资源，**全队共享**）。
 *
 * ⚠️ 挂载约定：**每队只有一份**，挂在「队伍共享 ASC」上（如队伍/队长 Actor 的 ASC），
 *    **不要挂到单个角色身上**。官方确认 Gradient 是全队共用的一池资源，
 *    显示为队伍头像框下方的共享分段条。
 *
 * 机制（官方资料）：
 *   - 填充：**消耗 AP 施放技能**会累积 Gradient；攒满一段得到 1 个 Gradient Charge（GC）。
 *   - 消耗：Gradient Attack（各角色终极技，按 1GC / 2GC / 3GC 计价）与 Gradient Counter。
 *   - 特点：使用 Gradient Attack **不会结束回合**（相当于一回合两次出手）。
 *
 * 因此这里用「充能数 + 当前段进度」建模，而不是单一百分比：
 *   - `GradientCharges`     ：已攒满的 GC 数（可直接消费）
 *   - `MaxGradientCharges`  ：GC 上限（官方未给出确切上限，按最高消耗 3GC 取默认 3，可配）
 *   - `GradientProgress`    ：当前正在攒的那一段的进度 [0, 1)
 */
UCLASS()
class GAMEWORK_API UCatGradientAttributeSet : public UCatAttributeSetBase
{
	GENERATED_BODY()

public:
	UCatGradientAttributeSet();

	/** 已攒满的 Gradient 充能数（GC），可直接消费 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_GradientCharges, Category = "Cat|Gradient")
	FGameplayAttributeData GradientCharges;
	ATTRIBUTE_ACCESSORS_BASIC(UCatGradientAttributeSet, GradientCharges);

	/** Gradient 充能上限（GC） */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxGradientCharges, Category = "Cat|Gradient")
	FGameplayAttributeData MaxGradientCharges;
	ATTRIBUTE_ACCESSORS_BASIC(UCatGradientAttributeSet, MaxGradientCharges);

	/** 当前正在积攒的那一段的进度 [0, 1) */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_GradientProgress, Category = "Cat|Gradient")
	FGameplayAttributeData GradientProgress;
	ATTRIBUTE_ACCESSORS_BASIC(UCatGradientAttributeSet, GradientProgress);

	/** 充能/进度变化广播 */
	UPROPERTY(BlueprintAssignable, Category = "Cat|Gradient")
	FCatGradientChangedDelegate OnGradientChanged;

	/** 攒满一格 GC 时广播（用于播提示音/特效） */
	UPROPERTY(BlueprintAssignable, Category = "Cat|Gradient")
	FCatGradientChargeGainedDelegate OnGradientChargeGained;

	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// ~begin UCatAttributeSetBase
	virtual void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const override;
	// ~end UCatAttributeSetBase

	UFUNCTION()
	void OnRep_GradientCharges(const FGameplayAttributeData& OldGradientCharges);

	UFUNCTION()
	void OnRep_MaxGradientCharges(const FGameplayAttributeData& OldMaxGradientCharges);

	UFUNCTION()
	void OnRep_GradientProgress(const FGameplayAttributeData& OldGradientProgress);
};
