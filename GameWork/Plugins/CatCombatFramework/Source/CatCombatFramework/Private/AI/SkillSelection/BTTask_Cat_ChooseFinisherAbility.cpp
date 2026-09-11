// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/SkillSelection/BTTask_Cat_ChooseFinisherAbility.h"

#include "GameFramework/Character.h"

#include "Combat/Component/SVCharacterTurnComponent.h"

UBTTask_Cat_ChooseFinisherAbility::UBTTask_Cat_ChooseFinisherAbility()
{
	NodeName = TEXT("Cat Choose Finisher Ability");
}

EBTNodeResult::Type UBTTask_Cat_ChooseFinisherAbility::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* /*NodeMemory*/)
{
	// 目标血量未到斩杀线则跳过终结技（取不到血量数据时不拦截，交后续可激活判定）
	if (ACharacter* Target = GetTargetActor(OwnerComp))
	{
		float HealthNormalized = 0.f;
		if (GetHealthNormalized(Target, HealthAttribute, MaxHealthAttribute, HealthNormalized)
			&& HealthNormalized > FinisherHealthThreshold)
		{
			return EBTNodeResult::Failed;
		}
	}

	USVCharacterTurnComponent* TurnComp = GetTurnComponent(OwnerComp);
	if (!TurnComp)
	{
		return EBTNodeResult::Failed;
	}

	FGameplayTag Tag;
	if (!PickFirstActivatableTag(OwnerComp, TurnComp->GetTurnRole(), FinisherAbilityTags, Tag))
	{
		return EBTNodeResult::Failed;
	}

	WriteSelectedAbilityTag(OwnerComp, Tag);
	return EBTNodeResult::Succeeded;
}

FString UBTTask_Cat_ChooseFinisherAbility::GetStaticDescription() const
{
	return FString::Printf(TEXT("终结技: %s | 阈值: %.2f"), *FinisherAbilityTags.ToStringSimple(), FinisherHealthThreshold);
}
