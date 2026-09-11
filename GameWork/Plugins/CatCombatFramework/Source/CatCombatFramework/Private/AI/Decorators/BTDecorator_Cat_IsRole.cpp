// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Decorators/BTDecorator_Cat_IsRole.h"

#include "AIController.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

UBTDecorator_Cat_IsRole::UBTDecorator_Cat_IsRole()
{
	NodeName = TEXT("Cat Is Role");
}

bool UBTDecorator_Cat_IsRole::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* /*NodeMemory*/) const
{
	if (!ExpectedStateTag.IsValid())
	{
		return false;
	}

	AAIController* AIOwner = OwnerComp.GetAIOwner();
	APawn* Pawn = AIOwner ? AIOwner->GetPawn() : nullptr;
	if (!Pawn)
	{
		return false;
	}

	// 移植说明：源项目经 ASVCharacterBase（GameplayTagAssetInterface）查询状态 Tag，本框架改查角色 ASC 的 LooseTag
	const UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn);
	return ASC && ASC->HasMatchingGameplayTag(ExpectedStateTag);
}

FString UBTDecorator_Cat_IsRole::GetStaticDescription() const
{
	return FString::Printf(TEXT("状态: %s"), *ExpectedStateTag.ToString());
}
