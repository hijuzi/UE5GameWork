// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/UBTTask_CatBase.h"

#include "AIController.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"

#include "AI/CatAIControllerBase.h"
#include "AI/CatAIControlData.h"
#include "Combat/Component/SVCharacterTurnComponent.h"
#include "Combat/SVCombatFunctionLibrary.h"

ACharacter* UBTTask_CatBase::GetOwnerCharacter(UBehaviorTreeComponent& OwnerComp)
{
	AAIController* AIOwner = OwnerComp.GetAIOwner();
	APawn* Pawn = AIOwner ? AIOwner->GetPawn() : nullptr;
	return Cast<ACharacter>(Pawn);
}

USVCharacterTurnComponent* UBTTask_CatBase::GetTurnComponent(UBehaviorTreeComponent& OwnerComp)
{
	return USVCharacterTurnComponent::GetSVCharacterTurnComponent(GetOwnerCharacter(OwnerComp));
}

UBlackboardComponent* UBTTask_CatBase::GetBlackboard(UBehaviorTreeComponent& OwnerComp)
{
	return OwnerComp.GetBlackboardComponent();
}

ACharacter* UBTTask_CatBase::GetTargetActor(UBehaviorTreeComponent& OwnerComp)
{
	UBlackboardComponent* BB = GetBlackboard(OwnerComp);
	UObject* Obj = BB ? BB->GetValueAsObject(TEXT("TargetActor")) : nullptr;
	return Cast<ACharacter>(Obj);
}

void UBTTask_CatBase::WriteTargetActor(UBehaviorTreeComponent& OwnerComp, AActor* Target)
{
	if (UBlackboardComponent* BB = GetBlackboard(OwnerComp))
	{
		BB->SetValueAsObject(TEXT("TargetActor"), Target);
	}
}

void UBTTask_CatBase::WriteSelectedAbilityTag(UBehaviorTreeComponent& OwnerComp, const FGameplayTag& Tag)
{
	if (UBlackboardComponent* BB = GetBlackboard(OwnerComp))
	{
		BB->SetValueAsName(TEXT("SelectedAbilityTag"), Tag.GetTagName());
	}
}

bool UBTTask_CatBase::PickFirstActivatableTag(UBehaviorTreeComponent& OwnerComp, ECombatTurnRole Role, const FGameplayTagContainer& Tags, FGameplayTag& OutTag)
{
	ACharacter* Character = GetOwnerCharacter(OwnerComp);
	if (!Character)
	{
		return false;
	}

	for (const FGameplayTag& Tag : Tags)
	{
		if (Tag.IsValid() && USVCombatFunctionLibrary::CanActivateTurnAbilityByTag(Character, Tag, Role))
		{
			OutTag = Tag;
			return true;
		}
	}

	return false;
}

UCatAIControlData* UBTTask_CatBase::GetAIControlData(UBehaviorTreeComponent& OwnerComp)
{
	ACatAIControllerBase* CatAIOwner = Cast<ACatAIControllerBase>(OwnerComp.GetAIOwner());
	return CatAIOwner ? CatAIOwner->GetAIControlData() : nullptr;
}

bool UBTTask_CatBase::HasMatchingTag(AActor* Actor, const FGameplayTag& Tag)
{
	if (!IsValid(Actor))
	{
		return false;
	}

	const UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor);
	return ASC && ASC->HasMatchingGameplayTag(Tag);
}

bool UBTTask_CatBase::GetHealthValue(AActor* Actor, const FGameplayAttribute& HealthAttribute, float& OutHealth)
{
	OutHealth = 0.f;
	if (!IsValid(Actor) || !HealthAttribute.IsValid())
	{
		return false;
	}

	const UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor);
	if (!ASC || !ASC->HasAttributeSetForAttribute(HealthAttribute))
	{
		return false;
	}

	OutHealth = ASC->GetNumericAttribute(HealthAttribute);
	return true;
}

bool UBTTask_CatBase::GetHealthNormalized(AActor* Actor, const FGameplayAttribute& HealthAttribute, const FGameplayAttribute& MaxHealthAttribute, float& OutNormalized)
{
	OutNormalized = 0.f;

	float Health = 0.f;
	float MaxHealth = 0.f;
	if (!GetHealthValue(Actor, HealthAttribute, Health) || !GetHealthValue(Actor, MaxHealthAttribute, MaxHealth) || MaxHealth <= 0.f)
	{
		return false;
	}

	OutNormalized = Health / MaxHealth;
	return true;
}
