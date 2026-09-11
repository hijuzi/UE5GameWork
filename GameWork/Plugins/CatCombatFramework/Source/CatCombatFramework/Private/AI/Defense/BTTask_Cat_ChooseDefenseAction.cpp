// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Defense/BTTask_Cat_ChooseDefenseAction.h"

#include "BehaviorTree/BehaviorTreeComponent.h"
#include "GameFramework/Character.h"

#include "AI/CatAIControlData.h"
#include "AI/CatAISkillDecisionData.h"
#include "Combat/Component/SVCharacterTurnComponent.h"
#include "Combat/SVCombatFunctionLibrary.h"

UBTTask_Cat_ChooseDefenseAction::UBTTask_Cat_ChooseDefenseAction()
{
	NodeName = TEXT("Cat Choose Defense Action");
}

FString UBTTask_Cat_ChooseDefenseAction::GetStaticDescription() const
{
	return TEXT("从 UCatAIControlData->SkillDecisionData 的 DefenderSkills 中加权随机选择防御行为。");
}

EBTNodeResult::Type UBTTask_Cat_ChooseDefenseAction::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* /*NodeMemory*/)
{
	// 读取 AI 控制数据中的防御技能权重表（挂在 AI 控制器 ACatAIControllerBase 上）
	UCatAIControlData* AIControlData = GetAIControlData(OwnerComp);
	UAISkillDecisionData* DecisionData = AIControlData ? AIControlData->SkillDecisionData : nullptr;
	if (!DecisionData)
	{
		return EBTNodeResult::Failed;
	}

	ACharacter* Character = GetOwnerCharacter(OwnerComp);
	USVCharacterTurnComponent* TurnComp = GetTurnComponent(OwnerComp);
	if (!Character || !TurnComp)
	{
		return EBTNodeResult::Failed;
	}

	const ECombatTurnRole Role = TurnComp->GetTurnRole();

	FGameplayTag Tag;
	if (!USVCombatFunctionLibrary::PickSkillFromWeightTable(Character, Role, DecisionData->DefenderSkills, Tag))
	{
		return EBTNodeResult::Failed;
	}

	// 写黑板
	WriteSelectedAbilityTag(OwnerComp, Tag);

	return EBTNodeResult::Succeeded;
}
