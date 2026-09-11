// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Action/BTTask_Cat_FinishTurn.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "GameFramework/Character.h"

#include "Combat/Component/SVCharacterTurnComponent.h"

UBTTask_Cat_FinishTurn::UBTTask_Cat_FinishTurn()
{
	NodeName = TEXT("Cat Finish Turn");
}

EBTNodeResult::Type UBTTask_Cat_FinishTurn::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* /*NodeMemory*/)
{
	AAIController* AIOwner = OwnerComp.GetAIOwner();
	ACharacter* Character = AIOwner ? Cast<ACharacter>(AIOwner->GetPawn()) : nullptr;
	if (USVCharacterTurnComponent* TurnComp = USVCharacterTurnComponent::GetSVCharacterTurnComponent(Character))
	{
		TurnComp->NotifyTurnFinished();
	}

	return EBTNodeResult::Succeeded;
}
