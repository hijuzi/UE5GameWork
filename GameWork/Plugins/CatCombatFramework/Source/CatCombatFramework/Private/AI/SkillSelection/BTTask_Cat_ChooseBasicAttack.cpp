// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/SkillSelection/BTTask_Cat_ChooseBasicAttack.h"

#include "Combat/Component/SVCharacterTurnComponent.h"

UBTTask_Cat_ChooseBasicAttack::UBTTask_Cat_ChooseBasicAttack()
{
	NodeName = TEXT("Cat Choose Basic Attack");
}

EBTNodeResult::Type UBTTask_Cat_ChooseBasicAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* /*NodeMemory*/)
{
	USVCharacterTurnComponent* TurnComp = GetTurnComponent(OwnerComp);
	if (!TurnComp)
	{
		return EBTNodeResult::Failed;
	}

	FGameplayTag Tag;
	if (!PickFirstActivatableTag(OwnerComp, TurnComp->GetTurnRole(), BasicAttackTags, Tag))
	{
		return EBTNodeResult::Failed;
	}

	WriteSelectedAbilityTag(OwnerComp, Tag);
	return EBTNodeResult::Succeeded;
}

FString UBTTask_Cat_ChooseBasicAttack::GetStaticDescription() const
{
	return FString::Printf(TEXT("普攻: %s"), *BasicAttackTags.ToStringSimple());
}
