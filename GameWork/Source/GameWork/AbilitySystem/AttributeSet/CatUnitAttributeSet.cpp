// Copyright Epic Games, Inc. All Rights Reserved.

#include "CatUnitAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

UCatUnitAttributeSet::UCatUnitAttributeSet()
{
	// ---- 生命组 ----
	InitHealth(100.f);
	InitMaxHealth(100.f);
	InitDamage(0.f);
	InitHealing(0.f);

	// ---- 护盾组 ----
	InitShieldLayers(0.f);
	InitMaxShieldLayers(0.f); // 默认无护盾能力，由技能 / Pictos 抬升
	InitShieldBreak(0.f);

	// ---- 进攻组 ----
	InitAttack(10.f);
	InitCritRate(0.05f);       // 5% 基础暴击
	InitCritDamage(1.5f);      // 官方固定 +50% → 150%
	InitDamageMultiplier(1.f); // 乘法单位元
	InitBreakPower(1.f);       // 乘法单位元

	// ---- 防御组 ----
	InitDamageReduction(0.f);
	InitParryEfficiency(1.f);  // 乘法单位元
	InitDodgeEfficiency(1.f);  // 乘法单位元

	// ---- 行动组 ----
	InitSpeed(100.f);          // 基准速度，具体值由 Agility 派生 GE 覆盖
}

// ======================================================================
// Clamp（按分组拆分，避免一个巨型 switch）
// ======================================================================

void UCatUnitAttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	ClampVital(Attribute, NewValue);
	ClampOffense(Attribute, NewValue);
	ClampDefense(Attribute, NewValue);
}

void UCatUnitAttributeSet::ClampVital(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		// 只保下限，不设上限 —— 上限由玩法 / 加点决定
		NewValue = FMath::Max(1.f, NewValue);
	}
	else if (Attribute == GetShieldLayersAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxShieldLayers());
	}
	else if (Attribute == GetMaxShieldLayersAttribute())
	{
		NewValue = FMath::Max(0.f, NewValue);
	}
	else if (Attribute == GetDamageAttribute()
		|| Attribute == GetHealingAttribute()
		|| Attribute == GetShieldBreakAttribute())
	{
		NewValue = FMath::Max(0.f, NewValue);
	}
}

void UCatUnitAttributeSet::ClampOffense(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetCritRateAttribute())
	{
		// 暴击率钳到 [0, 1]，超过 100% 无意义且会污染伤害期望计算
		NewValue = FMath::Clamp(NewValue, 0.f, 1.f);
	}
	else if (Attribute == GetCritDamageAttribute())
	{
		// 暴击倍率不得低于 1（暴击至少不亏）
		NewValue = FMath::Max(1.f, NewValue);
	}
	else if (Attribute == GetAttackAttribute()
		|| Attribute == GetDamageMultiplierAttribute()
		|| Attribute == GetBreakPowerAttribute())
	{
		NewValue = FMath::Max(0.f, NewValue);
	}
}

void UCatUnitAttributeSet::ClampDefense(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetDamageReductionAttribute())
	{
		// 上限必须为 1：减伤超过 100% 会让伤害变成负数（治疗敌人）
		NewValue = FMath::Clamp(NewValue, 0.f, 1.f);
	}
	else if (Attribute == GetParryEfficiencyAttribute()
		|| Attribute == GetDodgeEfficiencyAttribute()
		|| Attribute == GetSpeedAttribute())
	{
		NewValue = FMath::Max(0.f, NewValue);
	}
}

// ======================================================================
// 回调
// ======================================================================

void UCatUnitAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	// ---- 生命组 ----
	if (Attribute == GetMaxHealthAttribute() && GetHealth() > NewValue)
	{
		ClampCurrentValueToMax(GetHealthAttribute(), NewValue);
	}

	// 倒地 / 复活标记在此派生：PostAttributeChange 在客户端经 OnRep 重聚合时同样会触发，
	// 因此服务器与客户端状态天然一致，无需额外复制该标记。
	if (Attribute == GetHealthAttribute() || Attribute == GetMaxHealthAttribute())
	{
		OnHealthChanged.Broadcast(GetHealth(), GetMaxHealth());

		const bool bNowOutOfHealth = GetHealth() <= 0.f;
		if (bNowOutOfHealth != bOutOfHealth)
		{
			bOutOfHealth = bNowOutOfHealth;
			OnOutOfHealth.Broadcast(bOutOfHealth);
		}
	}

	// ---- 护盾组 ----
	if (Attribute == GetMaxShieldLayersAttribute() && GetShieldLayers() > NewValue)
	{
		ClampCurrentValueToMax(GetShieldLayersAttribute(), NewValue);
	}

	if (Attribute == GetShieldLayersAttribute() || Attribute == GetMaxShieldLayersAttribute())
	{
		OnShieldLayersChanged.Broadcast(GetShieldLayers(), GetMaxShieldLayers());

		// 由有盾变为 0 层才算破盾（本来就是 0 不重复广播）
		if (Attribute == GetShieldLayersAttribute() && OldValue > 0.f && NewValue <= 0.f)
		{
			OnShieldBroken.Broadcast();
		}
	}

	// ---- 行动组 ----
	if (Attribute == GetSpeedAttribute())
	{
		OnSpeedChanged.Broadcast(GetSpeed());
	}
}

void UCatUnitAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// 只做分发；实际换算逻辑在各自的 Resolve* 里，UI / 状态广播统一交给 PostAttributeChange
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		ResolveDamage();
	}
	else if (Data.EvaluatedData.Attribute == GetHealingAttribute())
	{
		ResolveHealing();
	}
	else if (Data.EvaluatedData.Attribute == GetShieldBreakAttribute())
	{
		ResolveShieldBreak();
	}
}

void UCatUnitAttributeSet::ResolveDamage()
{
	const float LocalDamage = GetDamage();
	SetDamage(0.f); // 元属性用后即焚

	if (LocalDamage > 0.f)
	{
		SetHealth(FMath::Clamp(GetHealth() - LocalDamage, 0.f, GetMaxHealth()));
	}
}

void UCatUnitAttributeSet::ResolveHealing()
{
	const float LocalHealing = GetHealing();
	SetHealing(0.f);

	if (LocalHealing > 0.f)
	{
		SetHealth(FMath::Clamp(GetHealth() + LocalHealing, 0.f, GetMaxHealth()));
	}
}

void UCatUnitAttributeSet::ResolveShieldBreak()
{
	// 官方机制：只有瞄准射击（Aim Shot）命中才写这个元属性 —— 普通攻击对带盾目标无效
	const float LocalShieldBreak = GetShieldBreak();
	SetShieldBreak(0.f);

	if (LocalShieldBreak > 0.f)
	{
		SetShieldLayers(FMath::Clamp(GetShieldLayers() - LocalShieldBreak, 0.f, GetMaxShieldLayers()));
	}
}

void UCatUnitAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 生命组
	DOREPLIFETIME_CONDITION_NOTIFY(UCatUnitAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCatUnitAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);

	// 护盾组
	DOREPLIFETIME_CONDITION_NOTIFY(UCatUnitAttributeSet, ShieldLayers, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCatUnitAttributeSet, MaxShieldLayers, COND_None, REPNOTIFY_Always);

	// 进攻组
	DOREPLIFETIME_CONDITION_NOTIFY(UCatUnitAttributeSet, Attack, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCatUnitAttributeSet, CritRate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCatUnitAttributeSet, CritDamage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCatUnitAttributeSet, DamageMultiplier, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCatUnitAttributeSet, BreakPower, COND_None, REPNOTIFY_Always);

	// 防御组
	DOREPLIFETIME_CONDITION_NOTIFY(UCatUnitAttributeSet, DamageReduction, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCatUnitAttributeSet, ParryEfficiency, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCatUnitAttributeSet, DodgeEfficiency, COND_None, REPNOTIFY_Always);

	// 行动组
	DOREPLIFETIME_CONDITION_NOTIFY(UCatUnitAttributeSet, Speed, COND_None, REPNOTIFY_Always);
}

void UCatUnitAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCatUnitAttributeSet, Health, OldHealth);
}

void UCatUnitAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCatUnitAttributeSet, MaxHealth, OldMaxHealth);
}

void UCatUnitAttributeSet::OnRep_ShieldLayers(const FGameplayAttributeData& OldShieldLayers)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCatUnitAttributeSet, ShieldLayers, OldShieldLayers);
}

void UCatUnitAttributeSet::OnRep_MaxShieldLayers(const FGameplayAttributeData& OldMaxShieldLayers)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCatUnitAttributeSet, MaxShieldLayers, OldMaxShieldLayers);
}

void UCatUnitAttributeSet::OnRep_Attack(const FGameplayAttributeData& OldAttack)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCatUnitAttributeSet, Attack, OldAttack);
}

void UCatUnitAttributeSet::OnRep_CritRate(const FGameplayAttributeData& OldCritRate)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCatUnitAttributeSet, CritRate, OldCritRate);
}

void UCatUnitAttributeSet::OnRep_CritDamage(const FGameplayAttributeData& OldCritDamage)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCatUnitAttributeSet, CritDamage, OldCritDamage);
}

void UCatUnitAttributeSet::OnRep_DamageMultiplier(const FGameplayAttributeData& OldDamageMultiplier)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCatUnitAttributeSet, DamageMultiplier, OldDamageMultiplier);
}

void UCatUnitAttributeSet::OnRep_BreakPower(const FGameplayAttributeData& OldBreakPower)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCatUnitAttributeSet, BreakPower, OldBreakPower);
}

void UCatUnitAttributeSet::OnRep_DamageReduction(const FGameplayAttributeData& OldDamageReduction)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCatUnitAttributeSet, DamageReduction, OldDamageReduction);
}

void UCatUnitAttributeSet::OnRep_ParryEfficiency(const FGameplayAttributeData& OldParryEfficiency)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCatUnitAttributeSet, ParryEfficiency, OldParryEfficiency);
}

void UCatUnitAttributeSet::OnRep_DodgeEfficiency(const FGameplayAttributeData& OldDodgeEfficiency)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCatUnitAttributeSet, DodgeEfficiency, OldDodgeEfficiency);
}

void UCatUnitAttributeSet::OnRep_Speed(const FGameplayAttributeData& OldSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCatUnitAttributeSet, Speed, OldSpeed);
}
