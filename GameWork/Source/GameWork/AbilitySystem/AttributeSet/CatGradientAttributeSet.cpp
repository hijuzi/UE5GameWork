// Copyright Epic Games, Inc. All Rights Reserved.

#include "CatGradientAttributeSet.h"

UCatGradientAttributeSet::UCatGradientAttributeSet()
{
	InitGradientCharges(0.f);
	InitMaxGradientCharges(3.f); // 最高消耗 3GC，按 3 格默认
	InitGradientProgress(0.f);
}

void UCatGradientAttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetGradientChargesAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxGradientCharges());
	}
	else if (Attribute == GetMaxGradientChargesAttribute())
	{
		NewValue = FMath::Max(0.f, NewValue);
	}
	else if (Attribute == GetGradientProgressAttribute())
	{
		// 段内进度固定落在 [0, 1)
		NewValue = FMath::Clamp(NewValue, 0.f, 1.f);
	}
}

void UCatGradientAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	// 上限被压低时，充能数不能超过新上限
	if (Attribute == GetMaxGradientChargesAttribute() && GetGradientCharges() > NewValue)
	{
		ClampCurrentValueToMax(GetGradientChargesAttribute(), NewValue);
	}

	if (Attribute == GetGradientChargesAttribute()
		|| Attribute == GetMaxGradientChargesAttribute()
		|| Attribute == GetGradientProgressAttribute())
	{
		OnGradientChanged.Broadcast(GetGradientCharges(), GetMaxGradientCharges(), GetGradientProgress());

		// 充能数上升 → 攒满一格
		if (Attribute == GetGradientChargesAttribute() && NewValue > OldValue)
		{
			OnGradientChargeGained.Broadcast();
		}
	}
}
