// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CatAttributeSetBase.h"
#include "AbilitySystemComponent.h"
#include "CatTurnResourceAttributeSet.generated.h"

/** AP 变化（供头像下方 AP 点格 UI 监听） */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCatAPChangedDelegate, float, NewAP, float, MaxAP);

/**
 * 回合资源属性集（L4：战斗节奏资源，**玩家角色专用，每角色一份**）。
 *
 * 只承载 AP（Action Points）—— 官方确认 AP 是**每个角色独立**的资源，显示为角色头像下方的点格。
 *
 * AP 收支（官方资料）：
 *   - 普通攻击 +1
 *   - **每次成功格挡（Parry）+1**（连击被挡几段就加几点）
 *   - 部分 Pictos / Lumina（如 Dodger、Perilous Parry）额外回 AP
 *   - Energy Tint 道具：给单个盟友 +3 或更多
 *   - 技能消耗 −3 ~ −7；瞄准射击每次 −1
 *
 * 注意：**Gradient 不在这里** —— 官方确认 Gradient 是**全队共享**资源，见 `UCatGradientAttributeSet`。
 * 本集只提供数据与约束；「回合开始回复多少」由挂在 TimeAxis.Round.Start 的 GE 驱动。
 */
UCLASS()
class GAMEWORK_API UCatTurnResourceAttributeSet : public UCatAttributeSetBase
{
	GENERATED_BODY()

public:
	UCatTurnResourceAttributeSet();

	/** 当前行动点 [0, MaxAP]（每角色独立） */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_AP, Category = "Cat|TurnResource")
	FGameplayAttributeData AP;
	ATTRIBUTE_ACCESSORS_BASIC(UCatTurnResourceAttributeSet, AP);

	/** 行动点上限（点格数） */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxAP, Category = "Cat|TurnResource")
	FGameplayAttributeData MaxAP;
	ATTRIBUTE_ACCESSORS_BASIC(UCatTurnResourceAttributeSet, MaxAP);

	/** AP 变化广播 */
	UPROPERTY(BlueprintAssignable, Category = "Cat|TurnResource")
	FCatAPChangedDelegate OnAPChanged;

	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// ~begin UCatAttributeSetBase
	virtual void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const override;
	// ~end UCatAttributeSetBase

	UFUNCTION()
	void OnRep_AP(const FGameplayAttributeData& OldAP);

	UFUNCTION()
	void OnRep_MaxAP(const FGameplayAttributeData& OldMaxAP);
};
