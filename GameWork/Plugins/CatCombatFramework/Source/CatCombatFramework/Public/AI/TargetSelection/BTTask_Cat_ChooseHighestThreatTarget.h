// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/UBTTask_CatBase.h"
#include "BTTask_Cat_ChooseHighestThreatTarget.generated.h"

/** 目标选择：按威胁值选最高者（威胁值系统未实现，当前占位返回 Failed 交由上游兜底） */
UCLASS(Category = "Cat AI", meta = (DisplayName = "Cat Choose Highest Threat Target"))
class CATCOMBATFRAMEWORK_API UBTTask_Cat_ChooseHighestThreatTarget : public UBTTask_CatBase
{
	GENERATED_BODY()

public:
	UBTTask_Cat_ChooseHighestThreatTarget();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
