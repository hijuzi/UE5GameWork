// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/UBTTask_CatBase.h"
#include "BTTask_Cat_ChooseMainCharacterTarget.generated.h"

/** 目标选择：兜底选玩家主战斗角色，写 TargetActor */
UCLASS(Category = "Cat AI", meta = (DisplayName = "Cat Choose Main Character Target"))
class CATCOMBATFRAMEWORK_API UBTTask_Cat_ChooseMainCharacterTarget : public UBTTask_CatBase
{
	GENERATED_BODY()

public:
	UBTTask_Cat_ChooseMainCharacterTarget();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
