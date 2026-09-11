// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/SkillSelection/BTTask_Cat_ForcedSkillPriority.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"

#include "AI/CatAIControlData.h"
#include "AI/CatAISkillDecisionData.h"
#include "Combat/SVCombatFunctionLibrary.h"

UBTTask_Cat_ForcedSkillPriority::UBTTask_Cat_ForcedSkillPriority()
{
	NodeName = TEXT("Cat Forced Skill Priority");
}

EBTNodeResult::Type UBTTask_Cat_ForcedSkillPriority::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* /*NodeMemory*/)
{
	UBlackboardComponent* BB = GetBlackboard(OwnerComp);
	ACharacter* Character = GetOwnerCharacter(OwnerComp);
	if (!BB || !Character)
	{
		return EBTNodeResult::Failed;
	}

	// 读数据资产配置（UAISkillDecisionData 基类字段）
	UAISkillDecisionData* DecisionData = nullptr;
	if (UCatAIControlData* AIControlData = GetAIControlData(OwnerComp))
	{
		DecisionData = AIControlData->SkillDecisionData;
	}

	if (!DecisionData || DecisionData->ForcedSkillRules.Num() == 0)
	{
		// 未配置必然触发规则：不参与，交下游常规决策
		return EBTNodeResult::Failed;
	}

	// 按 Priority 降序检测（拷贝排序，不修改数据资产数组）
	TArray<FAISkillForcedRule> Rules = DecisionData->ForcedSkillRules;
	Rules.Sort([](const FAISkillForcedRule& A, const FAISkillForcedRule& B)
	{
		return A.Priority > B.Priority;
	});

	for (const FAISkillForcedRule& Rule : Rules)
	{
		if (!Rule.SkillTag.IsValid())
		{
			continue;
		}

		// 状态命中：未指定 RequiredStateTag = 任意状态；指定则需角色当前持有该状态 Tag（如 Status.Health.Low）
		// 移植说明：源项目走 ASVCharacterBase 的 GameplayTagAssetInterface，本框架改查角色 ASC
		const bool bStateMatch = !Rule.RequiredStateTag.IsValid()
			|| HasMatchingTag(Character, Rule.RequiredStateTag);
		if (!bStateMatch)
		{
			continue;
		}

		// 技能可激活（按当前职责判定），可激活才必发；不可激活则跳过该规则看下一条
		if (!USVCombatFunctionLibrary::CanActivateTurnAbilityByTagWithCurrentRole(Character, Rule.SkillTag))
		{
			continue;
		}

		// 命中：强制选中该技能
		WriteSelectedAbilityTag(OwnerComp, Rule.SkillTag);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}

FString UBTTask_Cat_ForcedSkillPriority::GetStaticDescription() const
{
	return TEXT("必然触发：按 ForcedSkillRules 优先级降序检测（状态命中 + 可激活）→ 强制选 SkillTag（配置于 UAISkillDecisionData）");
}
