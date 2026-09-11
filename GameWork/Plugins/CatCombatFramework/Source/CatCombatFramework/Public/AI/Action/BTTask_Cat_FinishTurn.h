// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Cat_FinishTurn.generated.h"

/**
 * 结束回合 Task：调用回合组件 NotifyTurnFinished。
 */
UCLASS(Category = "Cat AI", meta = (DisplayName = "Cat Finish Turn"))
class CATCOMBATFRAMEWORK_API UBTTask_Cat_FinishTurn : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_Cat_FinishTurn();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
