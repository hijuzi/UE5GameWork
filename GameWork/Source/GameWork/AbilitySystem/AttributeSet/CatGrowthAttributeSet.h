// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CatAttributeSetBase.h"
#include "AbilitySystemComponent.h"
#include "CatGrowthAttributeSet.generated.h"

class UCurveFloat;

/**
 * 等级变化委托（仅在升级时广播）。
 * @param NewLevel 升级后的等级
 * @param OldLevel 升级前的等级
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCatLevelChangedDelegate, int32, NewLevel, int32, OldLevel);

/**
 * 成长属性集（L1 + L2：等级 / 经验 / 点数 / 加点属性）。
 *
 * 合并来源：原 `UCatProgressionAttributeSet` + `UCatPrimaryAttributeSet`。
 * 合并依据：二者的**挂载对象（角色）、阵营（玩家）、持久化与重置时机（存档、不重置）三条完全相同**，
 *          合并零代价；且合并后「发放（经验 → 发点）」与「消费（加点 → 写属性）」内聚到同一集，
 *          消除了「必须先挂 Progression 再挂 Primary」的跨集初始化顺序约束。
 *
 * 数据流：瞬时 GE 写元属性 `ExperienceGain` → `PostGameplayEffectExecute` 结算循环升级 →
 *        `Level` 变化触发 `OnLevelUp` → 加点界面消费 `AttributePoints` 写入五个加点属性。
 *
 * 持久化：本集整体随存档保留（战斗实时属性不在本集，因此不会把残血存进存档）。
 */
UCLASS()
class GAMEWORK_API UCatGrowthAttributeSet : public UCatAttributeSetBase
{
	GENERATED_BODY()

public:
	UCatGrowthAttributeSet();

	// ==================================================================
	// 等级 / 经验
	// ==================================================================

	/** 当前等级 [1, MaxLevel] */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Level, Category = "Cat|Growth|Progression")
	FGameplayAttributeData Level;
	ATTRIBUTE_ACCESSORS_BASIC(UCatGrowthAttributeSet, Level);

	/** 等级上限（官方 99） */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxLevel, Category = "Cat|Growth|Progression")
	FGameplayAttributeData MaxLevel;
	ATTRIBUTE_ACCESSORS_BASIC(UCatGrowthAttributeSet, MaxLevel);

	/** 当前等级内已积累的经验 [0, ExperienceToNextLevel) */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Experience, Category = "Cat|Growth|Progression")
	FGameplayAttributeData Experience;
	ATTRIBUTE_ACCESSORS_BASIC(UCatGrowthAttributeSet, Experience);

	/** 升到下一级所需经验（曲线 / DataTable 驱动，缺表时线性兜底） */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ExperienceToNextLevel, Category = "Cat|Growth|Progression")
	FGameplayAttributeData ExperienceToNextLevel;
	ATTRIBUTE_ACCESSORS_BASIC(UCatGrowthAttributeSet, ExperienceToNextLevel);

	/** 未分配的可加点数（官方：每级 3 点） */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_AttributePoints, Category = "Cat|Growth|Progression")
	FGameplayAttributeData AttributePoints;
	ATTRIBUTE_ACCESSORS_BASIC(UCatGrowthAttributeSet, AttributePoints);

	/** 未分配的可加技能点（官方：每级 1 点） */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_SkillPoints, Category = "Cat|Growth|Progression")
	FGameplayAttributeData SkillPoints;
	ATTRIBUTE_ACCESSORS_BASIC(UCatGrowthAttributeSet, SkillPoints);

	/** 元属性：经验入账。仅 GE execute 瞬间存在，结算后清零，不复制 */
	UPROPERTY(BlueprintReadOnly, Category = "Cat|Growth|Progression")
	FGameplayAttributeData ExperienceGain;
	ATTRIBUTE_ACCESSORS_BASIC(UCatGrowthAttributeSet, ExperienceGain);

	// ==================================================================
	// 加点属性（官方五大属性）
	// ==================================================================

	/** 活力：派生最大生命 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Vitality, Category = "Cat|Growth|Primary")
	FGameplayAttributeData Vitality;
	ATTRIBUTE_ACCESSORS_BASIC(UCatGrowthAttributeSet, Vitality);

	/** 力量：派生攻击力与武器补正 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Might, Category = "Cat|Growth|Primary")
	FGameplayAttributeData Might;
	ATTRIBUTE_ACCESSORS_BASIC(UCatGrowthAttributeSet, Might);

	/** 敏捷：派生速度（行动顺序） */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Agility, Category = "Cat|Growth|Primary")
	FGameplayAttributeData Agility;
	ATTRIBUTE_ACCESSORS_BASIC(UCatGrowthAttributeSet, Agility);

	/** 防御（加点属性）：派生减伤率。注意与 `UCatUnitAttributeSet` 的减伤率 `DamageReduction` 是两层概念 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Defense, Category = "Cat|Growth|Primary")
	FGameplayAttributeData Defense;
	ATTRIBUTE_ACCESSORS_BASIC(UCatGrowthAttributeSet, Defense);

	/** 运气：派生暴击率等收益 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Luck, Category = "Cat|Growth|Primary")
	FGameplayAttributeData Luck;
	ATTRIBUTE_ACCESSORS_BASIC(UCatGrowthAttributeSet, Luck);

	// ==================================================================
	// 配置
	// ==================================================================

	/** 每次升级发放的加点数（官方：3） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cat|Growth", meta = (ClampMin = "0"))
	int32 PointsPerLevel = 3;

	/** 每次升级发放的技能点数（官方：1） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cat|Growth", meta = (ClampMin = "0"))
	int32 SkillPointsPerLevel = 1;

	/** 线性兜底公式：1 级升 2 级所需经验 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cat|Growth", meta = (ClampMin = "1"))
	float BaseExperienceRequirement = 100.f;

	/** 线性兜底公式：每升一级所需经验的增量 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cat|Growth", meta = (ClampMin = "0"))
	float ExperienceIncrementPerLevel = 50.f;

	/** 可选：等级 → 升级所需经验 曲线（横轴为当前等级）。配置后优先于线性兜底公式 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cat|Growth")
	TObjectPtr<UCurveFloat> ExperienceCurve;

	/** 升级广播（供 UI、派生属性重算监听） */
	UPROPERTY(BlueprintAssignable, Category = "Cat|Growth")
	FCatLevelChangedDelegate OnLevelUp;

	// ==================================================================
	// 回调
	// ==================================================================

	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** 查询指定等级升到下一级所需经验（曲线优先，线性兜底） */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Cat|Growth")
	float ComputeExperienceToNextLevel(int32 InLevel) const;

protected:
	// ~begin UCatAttributeSetBase
	virtual void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const override;
	// ~end UCatAttributeSetBase

	/** 等级 / 经验 / 点数的约束 */
	void ClampProgression(const FGameplayAttribute& Attribute, float& NewValue) const;

	/** 五个加点属性的约束 */
	void ClampPrimary(const FGameplayAttribute& Attribute, float& NewValue) const;

	/** 经验入账结算：循环升级 + 溢出结转 + 发放点数 */
	void ResolveExperienceGain(float GainedExperience);

	UFUNCTION()
	void OnRep_Level(const FGameplayAttributeData& OldLevel);

	UFUNCTION()
	void OnRep_MaxLevel(const FGameplayAttributeData& OldMaxLevel);

	UFUNCTION()
	void OnRep_Experience(const FGameplayAttributeData& OldExperience);

	UFUNCTION()
	void OnRep_ExperienceToNextLevel(const FGameplayAttributeData& OldExperienceToNextLevel);

	UFUNCTION()
	void OnRep_AttributePoints(const FGameplayAttributeData& OldAttributePoints);

	UFUNCTION()
	void OnRep_SkillPoints(const FGameplayAttributeData& OldSkillPoints);

	UFUNCTION()
	void OnRep_Vitality(const FGameplayAttributeData& OldVitality);

	UFUNCTION()
	void OnRep_Might(const FGameplayAttributeData& OldMight);

	UFUNCTION()
	void OnRep_Agility(const FGameplayAttributeData& OldAgility);

	UFUNCTION()
	void OnRep_Defense(const FGameplayAttributeData& OldDefense);

	UFUNCTION()
	void OnRep_Luck(const FGameplayAttributeData& OldLuck);

private:
	/** 单次经验入账允许的最大升级次数，防止经验表配错（如所需经验为 0）导致死循环 */
	static constexpr int32 MaxLevelUpIterations = 128;
};
