// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CatAttributeSetBase.h"
#include "AbilitySystemComponent.h"
#include "CatTurnResourceAttributeSet.generated.h"

/** AP 变化（供头像下方 AP 点格 UI 监听） */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCatAPChangedDelegate, float, NewAP, float, MaxAP);

/**
 * 回合资源属性集（L4：战斗节奏资源，玩家角色专用，每角色一份）。
 *
 * 只承载 AP（Action Points）。AP 每个角色独立，显示在角色头像下方的点格。
 *
 * 收支：普通攻击 +1；每次成功格挡 +1；部分 Pictos / Lumina 额外回 AP；
 *      道具 Energy Tint 给单个盟友 +3 或更多；技能消耗 3~7；瞄准射击每次 −1。
 *
 * 设计意图是把防守表现变成进攻资源，格挡越多能用的技能越多。
 *
 * 注意：Gradient 不在这里。它是全队共享资源，见 `UCatGradientAttributeSet`。
 * 本集只提供数据与约束，「回合开始回复多少」由挂在 TimeAxis.Round.Start 的 GE 驱动。
 */
UCLASS()
class GAMEWORK_API UCatTurnResourceAttributeSet : public UCatAttributeSetBase
{
	GENERATED_BODY()

public:
	UCatTurnResourceAttributeSet();

	/** 当前行动点 [0, MaxAP] */
	UPROPERTY(BlueprintReadOnly, Category = "Cat|TurnResource")
	FGameplayAttributeData AP;
	ATTRIBUTE_ACCESSORS_BASIC(UCatTurnResourceAttributeSet, AP);

	/** 行动点上限（点格数） */
	UPROPERTY(BlueprintReadOnly, Category = "Cat|TurnResource")
	FGameplayAttributeData MaxAP;
	ATTRIBUTE_ACCESSORS_BASIC(UCatTurnResourceAttributeSet, MaxAP);

	/** AP 变化广播 */
	UPROPERTY(BlueprintAssignable, Category = "Cat|TurnResource")
	FCatAPChangedDelegate OnAPChanged;

	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

protected:
	// ~begin UCatAttributeSetBase
	virtual void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const override;
	// ~end UCatAttributeSetBase
};
