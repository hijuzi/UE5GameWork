// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/TargetSelection/BTTask_Cat_ChooseHighestThreatTarget.h"

UBTTask_Cat_ChooseHighestThreatTarget::UBTTask_Cat_ChooseHighestThreatTarget()
{
	NodeName = TEXT("Cat Choose Highest Threat Target");
}

EBTNodeResult::Type UBTTask_Cat_ChooseHighestThreatTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* /*NodeMemory*/)
{
	// TODO: 威胁值/仇恨系统尚未实现，暂返回 Failed，让 Selector 落到「主角色兜底」
	return EBTNodeResult::Failed;
}
