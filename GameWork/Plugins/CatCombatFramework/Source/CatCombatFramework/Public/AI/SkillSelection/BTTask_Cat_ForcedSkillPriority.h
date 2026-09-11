// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/UBTTask_CatBase.h"
#include "BTTask_Cat_ForcedSkillPriority.generated.h"

/**
 * 技能选择：必然触发（通用）——条件命中即必发，无视权重层。
 *
 * 配置来源：AIControlData->SkillDecisionData（UAISkillDecisionData 基类）-> ForcedSkillRules
 *   （TArray<FAISkillForcedRule>，每项 = SkillTag + RequiredStateTag + Priority）：
 *   - 空数组：未启用，直接 Failed 交下游常规决策；
 *   - 按 Priority 降序遍历：RequiredStateTag 命中（为空=任意状态命中）且技能可激活 → 写 SelectedAbilityTag = SkillTag；
 *   - 全部未命中 → Failed。
 *
 * 低血处决等场景即本 Task 的一条配置（如 RequiredStateTag=Status.Health.Low、SkillTag=大招蓄力）。
 */
UCLASS(Category = "Cat AI", meta = (DisplayName = "Cat Forced Skill Priority"))
class CATCOMBATFRAMEWORK_API UBTTask_Cat_ForcedSkillPriority : public UBTTask_CatBase
{
	GENERATED_BODY()

public:
	UBTTask_Cat_ForcedSkillPriority();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
};
