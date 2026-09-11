// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/SkillSelection/BTTask_Cat_ChooseRegularSkill.h"

#include "BehaviorTree/BehaviorTreeComponent.h"
#include "GameFramework/Character.h"

#include "AI/CatAIControlData.h"
#include "AI/CatAISkillDecisionData.h"
#include "AI/UBTTask_CatBase.h"
#include "Combat/Component/SVCharacterTurnComponent.h"
#include "Combat/SVCombatFunctionLibrary.h"

UBTTask_Cat_ChooseRegularSkill::UBTTask_Cat_ChooseRegularSkill()
{
	NodeName = TEXT("Cat Choose Regular Skill");
}

FString UBTTask_Cat_ChooseRegularSkill::GetStaticDescription() const
{
	return TEXT("从 UCatAIControlData->SkillDecisionData 两层技能表中按状态/权重选择常规技能，写入黑板键。");
}

EBTNodeResult::Type UBTTask_Cat_ChooseRegularSkill::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* /*NodeMemory*/)
{
	ACharacter* Character = UBTTask_CatBase::GetOwnerCharacter(OwnerComp);
	if (!Character)
	{
		return EBTNodeResult::Failed;
	}

	// 1) 读取 AI 控制数据与决策数据（挂在 AI 控制器 ACatAIControllerBase 上）
	UCatAIControlData* AIControlData = UBTTask_CatBase::GetAIControlData(OwnerComp);
	UAISkillDecisionData* DecisionData = AIControlData ? AIControlData->SkillDecisionData : nullptr;
	if (!DecisionData)
	{
		return EBTNodeResult::Failed;
	}

	USVCharacterTurnComponent* TurnComp = USVCharacterTurnComponent::GetSVCharacterTurnComponent(Character);
	if (!TurnComp)
	{
		return EBTNodeResult::Failed;
	}

	// 2) 行动位
	const ECombatTurnRole Role = TurnComp->GetTurnRole();

	// 3) 选表：第 1 次行动先走高优先级表，落空回退常规表；第 2 次行动只用常规表
	const bool bFirstAction = TurnComp->GetActionCount() == 0;

	FGameplayTag ChosenTag;
	bool bPicked = false;
	if (bFirstAction)
	{
		// 第 1 次行动：高优先级表优先，命中即用；否则回退常规表
		bPicked = USVCombatFunctionLibrary::PickSkillFromWeightTable(Character, Role, DecisionData->HighPrioritySkills, ChosenTag);
		if (!bPicked)
		{
			bPicked = USVCombatFunctionLibrary::PickSkillFromWeightTable(Character, Role, DecisionData->LowPrioritySkills, ChosenTag);
		}
	}
	else
	{
		// 第 2 次行动：只用常规优先级表
		bPicked = USVCombatFunctionLibrary::PickSkillFromWeightTable(Character, Role, DecisionData->LowPrioritySkills, ChosenTag);
	}

	if (!bPicked)
	{
		return EBTNodeResult::Failed;
	}

	// 4) 写黑板
	UBTTask_CatBase::WriteSelectedAbilityTag(OwnerComp, ChosenTag);

	return EBTNodeResult::Succeeded;
}
