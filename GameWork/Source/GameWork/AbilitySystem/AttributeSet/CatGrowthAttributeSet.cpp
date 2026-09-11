// Copyright Epic Games, Inc. All Rights Reserved.

#include "CatGrowthAttributeSet.h"

#include "Curves/CurveFloat.h"
#include "GameplayEffectExtension.h"
#include "GameWork.h"

UCatGrowthAttributeSet::UCatGrowthAttributeSet()
{
	// ---- 等级 / 经验 ----
	InitLevel(1.f);
	InitMaxLevel(99.f);
	InitExperience(0.f);
	InitExperienceToNextLevel(ComputeExperienceToNextLevel(1));
	InitAttributePoints(0.f);
	InitSkillPoints(0.f);
	InitExperienceGain(0.f);

	// ---- 加点属性（默认值，后续由 DataTable / 角色资产按等级初始化）----
	InitVitality(10.f);
	InitMight(10.f);
	InitAgility(10.f);
	InitDefense(10.f);
	InitLuck(10.f);
}

float UCatGrowthAttributeSet::ComputeExperienceToNextLevel(int32 InLevel) const
{
	const int32 SafeLevel = FMath::Max(1, InLevel);

	// 优先使用曲线（横轴为当前等级）
	if (ExperienceCurve)
	{
		const float CurveValue = ExperienceCurve->GetFloatValue(static_cast<float>(SafeLevel));
		if (CurveValue > 0.f)
		{
			return CurveValue;
		}
	}

	// 线性兜底：基础值 + 每级增量 × (等级 - 1)，保证单调递增且不会溢出
	return FMath::Max(1.f, BaseExperienceRequirement + ExperienceIncrementPerLevel * (SafeLevel - 1));
}

// ======================================================================
// Clamp
// ======================================================================

void UCatGrowthAttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	ClampProgression(Attribute, NewValue);
	ClampPrimary(Attribute, NewValue);
}

void UCatGrowthAttributeSet::ClampProgression(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetLevelAttribute())
	{
		// 等级始终落在 [1, MaxLevel]
		NewValue = FMath::Clamp(NewValue, 1.f, FMath::Max(1.f, GetMaxLevel()));
	}
	else if (Attribute == GetMaxLevelAttribute())
	{
		NewValue = FMath::Max(1.f, NewValue);
	}
	else if (Attribute == GetExperienceAttribute())
	{
		// 注意：这里只保下限，不把 Experience 钳到 ExperienceToNextLevel 之下 ——
		// 否则 while 循环的升级条件 (Experience >= ExperienceToNextLevel) 永远不会成立。
		NewValue = FMath::Max(0.f, NewValue);
	}
	else if (Attribute == GetExperienceToNextLevelAttribute())
	{
		NewValue = FMath::Max(0.f, NewValue);
	}
	else if (Attribute == GetAttributePointsAttribute()
		|| Attribute == GetSkillPointsAttribute()
		|| Attribute == GetExperienceGainAttribute())
	{
		NewValue = FMath::Max(0.f, NewValue);
	}
}

void UCatGrowthAttributeSet::ClampPrimary(const FGameplayAttribute& Attribute, float& NewValue) const
{
	// 五个加点属性共用同一条约束：不得为负（官方无硬上限，软上限随属性浮动）
	if (Attribute == GetVitalityAttribute()
		|| Attribute == GetMightAttribute()
		|| Attribute == GetAgilityAttribute()
		|| Attribute == GetDefenseAttribute()
		|| Attribute == GetLuckAttribute())
	{
		NewValue = FMath::Max(0.f, NewValue);
	}
}

// ======================================================================
// 回调
// ======================================================================

void UCatGrowthAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	// 只在等级上升时广播升级（降级/重置不广播）
	if (Attribute == GetLevelAttribute() && NewValue > OldValue)
	{
		OnLevelUp.Broadcast(static_cast<int32>(NewValue), static_cast<int32>(OldValue));
	}
}

void UCatGrowthAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute != GetExperienceGainAttribute())
	{
		return;
	}

	const float GainedExperience = GetExperienceGain();
	SetExperienceGain(0.f); // 元属性用后即焚，避免重复结算

	if (GainedExperience > 0.f)
	{
		ResolveExperienceGain(GainedExperience);
	}
}

void UCatGrowthAttributeSet::ResolveExperienceGain(float GainedExperience)
{
	SetExperience(GetExperience() + GainedExperience);

	// 循环升级：一次经验入账可能跨多级；必须受 Level < MaxLevel 与迭代上限双重保护
	int32 Iterations = 0;
	while (GetLevel() < GetMaxLevel()
		&& GetExperienceToNextLevel() > 0.f
		&& GetExperience() >= GetExperienceToNextLevel()
		&& Iterations < MaxLevelUpIterations)
	{
		++Iterations;

		const float RequiredExperience = GetExperienceToNextLevel();

		// 先扣经验（溢出结转），再升级；SetLevel 会触发 PostAttributeChange → OnLevelUp
		SetExperience(GetExperience() - RequiredExperience);
		SetLevel(GetLevel() + 1);
		SetExperienceToNextLevel(ComputeExperienceToNextLevel(static_cast<int32>(GetLevel())));
		SetAttributePoints(GetAttributePoints() + static_cast<float>(PointsPerLevel));
		SetSkillPoints(GetSkillPoints() + static_cast<float>(SkillPointsPerLevel));
	}

	if (Iterations >= MaxLevelUpIterations)
	{
		UE_LOG(LogGameWork, Warning,
			TEXT("[CatGrowth] 升级循环达到迭代上限 %d，请检查 ExperienceCurve / 经验公式配置。"),
			MaxLevelUpIterations);
	}
}
