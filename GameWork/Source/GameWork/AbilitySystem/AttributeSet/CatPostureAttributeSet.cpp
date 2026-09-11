// Copyright Epic Games, Inc. All Rights Reserved.

#include "CatPostureAttributeSet.h"

#include "Net/UnrealNetwork.h"

UCatPostureAttributeSet::UCatPostureAttributeSet()
{
	InitBreak(0.f);
	InitMaxBreak(100.f);
	InitBreakRegen(0.f);
	InitStunTurns(0.f);
}

void UCatPostureAttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetBreakAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxBreak());
	}
	else if (Attribute == GetMaxBreakAttribute())
	{
		NewValue = FMath::Max(1.f, NewValue);
	}
	else if (Attribute == GetBreakRegenAttribute() || Attribute == GetStunTurnsAttribute())
	{
		NewValue = FMath::Max(0.f, NewValue);
	}
}

void UCatPostureAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (Attribute == GetBreakAttribute() || Attribute == GetMaxBreakAttribute())
	{
		OnBreakChanged.Broadcast(GetBreak(), GetMaxBreak());

		const bool bNowReady = GetBreak() >= GetMaxBreak() && GetMaxBreak() > 0.f;

		// 只在「首次达到满值」时广播一次，避免每点伤害都刷
		if (bNowReady && !bBreakReady)
		{
			bBreakReady = true;
			OnBreakReady.Broadcast();
		}
		else if (!bNowReady && bBreakReady)
		{
			// 被回复 / 上限抬高后回落，复位可引爆标记
			bBreakReady = false;
		}
	}
}

void UCatPostureAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UCatPostureAttributeSet, Break, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCatPostureAttributeSet, MaxBreak, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCatPostureAttributeSet, BreakRegen, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCatPostureAttributeSet, StunTurns, COND_None, REPNOTIFY_Always);
}

void UCatPostureAttributeSet::OnRep_Break(const FGameplayAttributeData& OldBreak)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCatPostureAttributeSet, Break, OldBreak);
}

void UCatPostureAttributeSet::OnRep_MaxBreak(const FGameplayAttributeData& OldMaxBreak)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCatPostureAttributeSet, MaxBreak, OldMaxBreak);
}

void UCatPostureAttributeSet::OnRep_BreakRegen(const FGameplayAttributeData& OldBreakRegen)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCatPostureAttributeSet, BreakRegen, OldBreakRegen);
}

void UCatPostureAttributeSet::OnRep_StunTurns(const FGameplayAttributeData& OldStunTurns)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCatPostureAttributeSet, StunTurns, OldStunTurns);
}
