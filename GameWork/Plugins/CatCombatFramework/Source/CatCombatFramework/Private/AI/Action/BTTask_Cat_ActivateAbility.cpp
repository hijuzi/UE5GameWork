// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Action/BTTask_Cat_ActivateAbility.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"

#include "Combat/Component/SVCharacterTurnComponent.h"

UBTTask_Cat_ActivateAbility::UBTTask_Cat_ActivateAbility()
{
	NodeName = TEXT("Cat Activate Ability");
}

EBTNodeResult::Type UBTTask_Cat_ActivateAbility::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* /*NodeMemory*/)
{
	AAIController* AIOwner = OwnerComp.GetAIOwner();
	ACharacter* Character = AIOwner ? Cast<ACharacter>(AIOwner->GetPawn()) : nullptr;
	USVCharacterTurnComponent* TurnComp = USVCharacterTurnComponent::GetSVCharacterTurnComponent(Character);
	if (!TurnComp)
	{
		return EBTNodeResult::Failed;
	}

	// 读黑板 SelectedAbilityTag（Name 键）
	FName TagName = NAME_None;
	if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
	{
		TagName = BB->GetValueAsName(TEXT("SelectedAbilityTag"));
	}
	if (TagName == NAME_None)
	{
		return EBTNodeResult::Failed;
	}

	const FGameplayTag AbilityTag = FGameplayTag::RequestGameplayTag(TagName);
	const ECombatTurnRole Role = TurnComp->GetTurnRole();

	return TurnComp->TryActivateActionByTag(AbilityTag, Role)
		? EBTNodeResult::Succeeded
		: EBTNodeResult::Failed;
}

void UBTTask_Cat_ActivateAbility::DescribeRuntimeValues(const UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTDescriptionVerbosity::Type Verbosity, TArray<FString>& Values) const
{
	Super::DescribeRuntimeValues(OwnerComp, NodeMemory, Verbosity, Values);

	if (const UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
	{
		const FName TagName = BB->GetValueAsName(TEXT("SelectedAbilityTag"));
		Values.Add(FString::Printf(TEXT("选中技能: %s"), *TagName.ToString()));
	}
}
