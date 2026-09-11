// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "CatAISkillDecisionData.generated.h"

/** 目标属性比较条件 */
UENUM(BlueprintType)
enum class EAISkillTargetAttrCompare : uint8
{
	Greater UMETA(DisplayName = "大于"),
	Less    UMETA(DisplayName = "小于"),
	Equal   UMETA(DisplayName = "等于"),
};

/**
 * 单个技能在某个血量段下的 AI 决策配置（相对权重参与该层随机）。
 */
USTRUCT(BlueprintType)
struct FAISkillWeight
{
	GENERATED_BODY()

	/** 技能 AbilityTag */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Skill")
	FGameplayTag SkillTag;

	/** 触发该技能需要满足的状态 Tag（Status.* 命名空间，如血量段 Status.Health.Low）；为空则忽略状态判定 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Skill", meta = (Categories = "Status"))
	FGameplayTag RequiredStateTag;

	/** 相对权重（同一层、同一血量段内归一化后随机） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Skill", meta = (ClampMin = "0.0"))
	float Weight = 1.0f;
};

/**
 * 必然触发（强制）技能规则：当 Owner 命中 RequiredStateTag 时，无条件强制选择 SkillTag（无视权重层）。
 * 与 FAISkillWeight（概率权重随机）相对：Weight = 概率触发；ForcedRule = 条件必发。
 * 供通用特殊规则 Task（BT_Cat_ForcedSkillPriority）按「优先级降序」逐条检测，多规则同时命中时取最高优先级。
 */
USTRUCT(BlueprintType)
struct FAISkillForcedRule
{
	GENERATED_BODY()

	/** 条件命中后强制触发的技能 AbilityTag */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Skill")
	FGameplayTag SkillTag;

	/** 触发该规则需要满足的状态 Tag（Status.* 命名空间，如 Status.Health.Low）；为空则视为任意状态均命中 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Skill", meta = (Categories = "Status"))
	FGameplayTag RequiredStateTag;

	/** 优先级（数值越大越先检测；多规则同时命中时取最高优先级规则） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Skill", meta = (ClampMin = "0"))
	int32 Priority = 0;
};

/**
 * 目标属性触发技能规则（通用）：Owner 满足 RequiredStateTag、且 TargetActor 的 TargetAttribute 满足比较条件时，
 * 按 HitChance 概率命中 → 强制选 SkillTag。供 BT_Cat_TargetAttrSkillPriority 消费。
 * 原「失衡快击二连」可表达为：TargetAttribute=CurrentBalance、比较=大于、阈值、SkillTag=普攻（快击出招由其它规则指定）。
 */
USTRUCT(BlueprintType)
struct FAISkillTargetAttrRule
{
	GENERATED_BODY()

	/** 规则描述（策划可读，仅作说明） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Skill")
	FString Description;

	/** 概率命中后强制触发的技能 AbilityTag */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Skill")
	FGameplayTag SkillTag;

	/** 触发本规则前 Owner 需满足的状态 Tag（Status.*）；为空 = 不校验自身状态 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Skill", meta = (Categories = "Status"))
	FGameplayTag RequiredStateTag;

	/** 命中概率（0~1，默认 1 = 必中）：目标属性条件满足后再掷一次 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Skill", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HitChance = 1.0f;

	/** 读取 TargetActor 的目标属性（如自定义 AttributeSet 的 CurrentBalance） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Skill")
	FGameplayAttribute TargetAttribute;

	/** 与目标属性比较的阈值 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Skill")
	float ThresholdValue = 0.5f;

	/** 比较条件 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Skill")
	EAISkillTargetAttrCompare CompareOp = EAISkillTargetAttrCompare::Greater;

	/** 优先级（数值越大越先检测；多条规则同时命中时取最高优先级规则） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Skill", meta = (ClampMin = "0"))
	int32 Priority = 0;
};

/**
 * 敌人技能 AI 决策数据资产（基类）。
 *
 * 保留通用两层技能表（高优先级 + 常规优先级），承载「何时选什么、权重多少」的配置；
 * Boss 专属字段（处决线、蓄力打断、失衡连击、防御闪避等）下沉到子类（如 UQueenSkillDecisionData）。
 */
UCLASS(BlueprintType, Blueprintable)
class CATCOMBATFRAMEWORK_API UAISkillDecisionData : public UDataAsset
{
	GENERATED_BODY()

public:
	/** 高优先级技能：第 1 次行动优先尝试（加权随机） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Skill")
	TArray<FAISkillWeight> HighPrioritySkills;

	/** 常规优先级技能：第 1 次行动高优落空时兜底，也是第 2 次行动的唯一来源 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Skill")
	TArray<FAISkillWeight> LowPrioritySkills;

    /** 防御阶段行动，（加权随机） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Defender")
	TArray<FAISkillWeight> DefenderSkills;

	/** 周期保底强制技能（通用）：每 PeriodicSkillInterval 次行动强制触发一次该技能（女王召唤随从即把此配为 SummonRetinue）；空 Tag 表示未启用 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Skill|Periodic")
	FGameplayTag PeriodicSkillTag;

	/** 周期保底触发间隔（行动次数，默认 4）：每累计 N 次行动在决策时强制触发 PeriodicSkillTag（见 BT_Cat_PeriodicSkillPriority） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Skill|Periodic", meta = (ClampMin = "1", EditCondition = "PeriodicSkillTag.IsValid()", EditConditionHides))
	int32 PeriodicSkillInterval = 4;

	/** 必然触发技能规则表（通用）：逐条按「优先级降序」检测 RequiredStateTag 命中且技能可激活 → 强制选 SkillTag；
	 *  空数组表示未启用该机制（BT_Cat_ForcedSkillPriority 直接 Failed，走常规决策）。
	 *  女王「低血处决」（07 §2.7 规则 1）即本表的一条配置：SkillTag=大招蓄力、RequiredStateTag=Status.Health.Low。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Skill|Forced")
	TArray<FAISkillForcedRule> ForcedSkillRules;

	/** 目标属性触发技能规则表（通用，BT_Cat_TargetAttrSkillPriority 消费）：逐条检测自身状态命中 + 目标属性满足比较条件
	 *  → 按 HitChance 概率命中则强制选 SkillTag；空数组表示未启用该机制 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Skill|TargetAttr")
	TArray<FAISkillTargetAttrRule> TargetAttrSkillRules;
};
