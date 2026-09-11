// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/SkillSelection/BTTask_Cat_TargetAttrSkillPriority.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"

#include "AI/CatAIControlData.h"
#include "AI/CatAISkillDecisionData.h"

UBTTask_Cat_TargetAttrSkillPriority::UBTTask_Cat_TargetAttrSkillPriority()
{
	NodeName = TEXT("Cat Target Attr Skill Priority");
}

EBTNodeResult::Type UBTTask_Cat_TargetAttrSkillPriority::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* /*NodeMemory*/)
{
	UBlackboardComponent* BB = GetBlackboard(OwnerComp);
	ACharacter* Character = GetOwnerCharacter(OwnerComp);
	ACharacter* Target = GetTargetActor(OwnerComp);
	if (!BB || !Character || !Target)
	{
		return EBTNodeResult::Failed;
	}

	// 读数据资产配置（UAISkillDecisionData 基类字段）
	UAISkillDecisionData* DecisionData = nullptr;
	if (UCatAIControlData* AIControlData = GetAIControlData(OwnerComp))
	{
		DecisionData = AIControlData->SkillDecisionData;
	}

	if (!DecisionData || DecisionData->TargetAttrSkillRules.Num() == 0)
	{
		// 未配置目标属性触发规则：不参与，交下游常规决策
		return EBTNodeResult::Failed;
	}

	// 目标 ASC（读取目标属性）
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
	if (!TargetASC)
	{
		return EBTNodeResult::Failed;
	}

	// 按 Priority 降序检测（拷贝排序，不修改数据资产数组）
	TArray<FAISkillTargetAttrRule> Rules = DecisionData->TargetAttrSkillRules;
	Rules.Sort([](const FAISkillTargetAttrRule& A, const FAISkillTargetAttrRule& B)
	{
		return A.Priority > B.Priority;
	});

	for (const FAISkillTargetAttrRule& Rule : Rules)
	{
		if (!Rule.SkillTag.IsValid() || !Rule.TargetAttribute.IsValid())
		{
			continue;
		}

		// 自身状态命中：RequiredStateTag 为空 = 不校验
		// 移植说明：源项目走 ASVCharacterBase 的 GameplayTagAssetInterface，本框架改查角色 ASC
		if (Rule.RequiredStateTag.IsValid() && !HasMatchingTag(Character, Rule.RequiredStateTag))
		{
			continue;
		}

		// 目标属性比较（目标无该属性集时按 0 处理，与 ASC::GetNumericAttribute 语义一致）
		const float AttrValue = TargetASC->GetNumericAttribute(Rule.TargetAttribute);
		bool bAttrMatch = false;
		switch (Rule.CompareOp)
		{
		case EAISkillTargetAttrCompare::Greater:
			bAttrMatch = AttrValue > Rule.ThresholdValue;
			break;
		case EAISkillTargetAttrCompare::Less:
			bAttrMatch = AttrValue < Rule.ThresholdValue;
			break;
		case EAISkillTargetAttrCompare::Equal:
			bAttrMatch = FMath::IsNearlyEqual(AttrValue, Rule.ThresholdValue);
			break;
		default:
			break;
		}

		if (!bAttrMatch)
		{
			continue;
		}

		// 概率命中：HitChance（0~1）
		if (FMath::FRand() > Rule.HitChance)
		{
			continue;
		}

		// 命中：强制选中该技能
		WriteSelectedAbilityTag(OwnerComp, Rule.SkillTag);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}

FString UBTTask_Cat_TargetAttrSkillPriority::GetStaticDescription() const
{
	return TEXT("目标属性触发：TargetAttrSkillRules 逐条检测（自身状态 + 目标属性比较）→ 概率命中强制选 SkillTag（配置于 UAISkillDecisionData）");
}
