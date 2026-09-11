// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/UBTTask_CatBase.h"
#include "BTTask_Cat_TargetAttrSkillPriority.generated.h"

/**
 * 技能选择：目标属性触发（通用）——Owner 状态命中 + TargetActor 目标属性满足比较条件 → 按 HitChance 概率命中则强制选 SkillTag。
 *
 * 配置来源：AIControlData->SkillDecisionData（UAISkillDecisionData 基类）-> TargetAttrSkillRules
 *   （TArray<FAISkillTargetAttrRule>，每项 = SkillTag + RequiredStateTag + HitChance + TargetAttribute + ThresholdValue + CompareOp + Priority）：
 *   - 空数组：未启用，直接 Failed 交下游常规决策；
 *   - 按 Priority 降序逐条检测：RequiredStateTag 命中（为空=不校验自身状态）且 TargetActor 的目标属性满足比较条件 →
 *     掷 HitChance 概率命中 → 写 SelectedAbilityTag = SkillTag；
 *   - 全部未命中 / 概率未命中 → Failed。
 *
 * 原「失衡快击二连」可表达为一条规则（如 CurrentBalance > 阈值），快击出招/跨行动延续由其它规则实现。
 */
UCLASS(Category = "Cat AI", meta = (DisplayName = "Cat Target Attr Skill Priority"))
class CATCOMBATFRAMEWORK_API UBTTask_Cat_TargetAttrSkillPriority : public UBTTask_CatBase
{
	GENERATED_BODY()

public:
	UBTTask_Cat_TargetAttrSkillPriority();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
};
