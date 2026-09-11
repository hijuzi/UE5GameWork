// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/TargetSelection/BTTask_Cat_ChooseMainCharacterTarget.h"

#include "GameFramework/Character.h"

#include "Combat/SVCombatFunctionLibrary.h"

UBTTask_Cat_ChooseMainCharacterTarget::UBTTask_Cat_ChooseMainCharacterTarget()
{
	NodeName = TEXT("Cat Choose Main Character Target");
}

EBTNodeResult::Type UBTTask_Cat_ChooseMainCharacterTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* /*NodeMemory*/)
{
	ACharacter* Owner = GetOwnerCharacter(OwnerComp);
	if (!Owner)
	{
		return EBTNodeResult::Failed;
	}

	// 兜底选 Owner 所在阵营的敌方主战斗角色（阵营判定已封装在 USVCombatFunctionLibrary）
	ACharacter* Main = USVCombatFunctionLibrary::GetMainOpponentCombatCharacter(Owner);
	if (!Main)
	{
		return EBTNodeResult::Failed;
	}

	WriteTargetActor(OwnerComp, Main);
	return EBTNodeResult::Succeeded;
}
