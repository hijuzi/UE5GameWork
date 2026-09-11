// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/UBTTask_CatBase.h"
#include "BTTask_Cat_ChooseDefenseAction.generated.h"

/**
 * 防御选择：从 UCatAIControlData->SkillDecisionData 的 DefenderSkills 权重表中
 * 按「状态 Tag + 加权随机」选一个防御行为，写 SelectedAbilityTag。
 */
UCLASS(Category = "Cat AI", meta = (DisplayName = "Cat Choose Defense Action"))
class CATCOMBATFRAMEWORK_API UBTTask_Cat_ChooseDefenseAction : public UBTTask_CatBase
{
	GENERATED_BODY()

public:
	UBTTask_Cat_ChooseDefenseAction();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
};
