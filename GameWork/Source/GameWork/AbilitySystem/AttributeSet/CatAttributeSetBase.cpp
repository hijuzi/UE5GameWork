// Copyright Epic Games, Inc. All Rights Reserved.

#include "CatAttributeSetBase.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"

void UCatAttributeSetBase::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	ClampAttribute(Attribute, NewValue);
}

void UCatAttributeSetBase::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
	ClampAttribute(Attribute, NewValue);
}

void UCatAttributeSetBase::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	// 默认不做任何约束，由子类按各自属性重写
}

void UCatAttributeSetBase::ClampCurrentValueToMax(const FGameplayAttribute& CurrentAttribute, float NewMaxValue) const
{
	if (!CurrentAttribute.IsValid())
	{
		return;
	}

	if (UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent())
	{
		ASC->ApplyModToAttribute(CurrentAttribute, EGameplayModOp::Override, NewMaxValue);
	}
}
