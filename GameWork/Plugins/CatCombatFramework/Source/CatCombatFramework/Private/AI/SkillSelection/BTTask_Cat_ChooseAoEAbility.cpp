// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/SkillSelection/BTTask_Cat_ChooseAoEAbility.h"

#include "Combat/Component/SVCharacterTurnComponent.h"

UBTTask_Cat_ChooseAoEAbility::UBTTask_Cat_ChooseAoEAbility()
{
	NodeName = TEXT("Cat Choose AoE Ability");
}

EBTNodeResult::Type UBTTask_Cat_ChooseAoEAbility::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* /*NodeMemory*/)
{
	USVCharacterTurnComponent* TurnComp = GetTurnComponent(OwnerComp);
	if (!TurnComp)
	{
		return EBTNodeResult::Failed;
	}

	// TODO: 多目标判断（TargetActors.Num() > 1）后续接入，当前仅按可激活选 AOE Tag
	FGameplayTag Tag;
	if (!PickFirstActivatableTag(OwnerComp, TurnComp->GetTurnRole(), AoEAbilityTags, Tag))
	{
		return EBTNodeResult::Failed;
	}

	WriteSelectedAbilityTag(OwnerComp, Tag);
	return EBTNodeResult::Succeeded;
}

FString UBTTask_Cat_ChooseAoEAbility::GetStaticDescription() const
{
	return FString::Printf(TEXT("AOE: %s"), *AoEAbilityTags.ToStringSimple());
}
