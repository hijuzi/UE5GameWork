// Copyright Epic Games, Inc. All Rights Reserved.

#include "CatTurnResourceAttributeSet.h"

UCatTurnResourceAttributeSet::UCatTurnResourceAttributeSet()
{
	InitAP(0.f);
	InitMaxAP(5.f); // 起始 AP 约 3~5（技能单次消耗 3~7）
}

void UCatTurnResourceAttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetAPAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxAP());
	}
	else if (Attribute == GetMaxAPAttribute())
	{
		NewValue = FMath::Max(0.f, NewValue);
	}
}

void UCatTurnResourceAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	// 上限被压低时，当前值不能超过新上限
	if (Attribute == GetMaxAPAttribute() && GetAP() > NewValue)
	{
		ClampCurrentValueToMax(GetAPAttribute(), NewValue);
	}

	if (Attribute == GetAPAttribute() || Attribute == GetMaxAPAttribute())
	{
		OnAPChanged.Broadcast(GetAP(), GetMaxAP());
	}
}
