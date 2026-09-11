// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Cat_ChooseRegularSkill.generated.h"

/**
 * 常规技能选择 Task（通用框架）。
 *
 * 从 UCatAIControlData->SkillDecisionData 的两层技能表中，按「状态 + 行动位」过滤候选，
 * 加权随机选出一个技能，写入黑板键（默认 SelectedAbilityTag）。
 */
UCLASS(Blueprintable, Category = "Cat AI", meta = (DisplayName = "Cat Choose Regular Skill"))
class CATCOMBATFRAMEWORK_API UBTTask_Cat_ChooseRegularSkill : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_Cat_ChooseRegularSkill();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
};
