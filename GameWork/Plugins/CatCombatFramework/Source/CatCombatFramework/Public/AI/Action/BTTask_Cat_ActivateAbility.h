// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Cat_ActivateAbility.generated.h"

/**
 * 激活能力 Task：读黑板 SelectedAbilityTag（Name），按当前职责激活对应 GA。
 */
UCLASS(Category = "Cat AI", meta = (DisplayName = "Cat Activate Ability"))
class CATCOMBATFRAMEWORK_API UBTTask_Cat_ActivateAbility : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_Cat_ActivateAbility();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void DescribeRuntimeValues(const UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTDescriptionVerbosity::Type Verbosity, TArray<FString>& Values) const override;
};
