// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Action/BTTask_Cat_EndTurnAction.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameplayTagContainer.h"

#include "Combat/SVCombatFunctionLibrary.h"
#include "Combat/SVCombatManagerSubsystem.h"
#include "Engine/GameInstance.h"
#include "GameFramework/Character.h"

UBTTask_Cat_EndTurnAction::UBTTask_Cat_EndTurnAction()
{
	NodeName = TEXT("Cat End Turn Action");
}

EBTNodeResult::Type UBTTask_Cat_EndTurnAction::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* /*NodeMemory*/)
{
	AAIController* AIOwner = OwnerComp.GetAIOwner();
	ACharacter* Character = AIOwner ? AIOwner->GetCharacter() : nullptr;
	if (!Character)
	{
		return EBTNodeResult::Failed;
	}

	// 读黑板当前选择的技能 Tag
	FName TagName = NAME_None;
	if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
	{
		TagName = BB->GetValueAsName(TEXT("SelectedAbilityTag"));
	}

	const FGameplayTag AbilityTag = FGameplayTag::RequestGameplayTag(TagName);

	// 当前选择的技能可激活：不结束回合，交后续激活流程执行
	if (AbilityTag.IsValid() && USVCombatFunctionLibrary::CanActivateTurnAbilityByTagWithCurrentRole(Character, AbilityTag))
	{
		return EBTNodeResult::Failed;
	}

	// 未选择技能或技能不可激活：经战斗管理器推进战斗回合（协调器判断切队伍或进入下一回合）
	USVCombatManagerSubsystem* CombatManager = Character->GetGameInstance() ? Character->GetGameInstance()->GetSubsystem<USVCombatManagerSubsystem>() : nullptr;
	if (!CombatManager)
	{
		return EBTNodeResult::Failed;
	}

	CombatManager->AdvanceCombatTurn();
	return EBTNodeResult::Succeeded;
}
