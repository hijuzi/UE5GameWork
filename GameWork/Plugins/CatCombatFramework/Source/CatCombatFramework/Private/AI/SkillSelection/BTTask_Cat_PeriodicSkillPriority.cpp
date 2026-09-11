// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/SkillSelection/BTTask_Cat_PeriodicSkillPriority.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"

#include "AI/CatAIControlData.h"
#include "AI/CatAISkillDecisionData.h"
#include "Combat/SVCombatFunctionLibrary.h"

const FName UBTTask_Cat_PeriodicSkillPriority::BBKey_PeriodicSkillCounter(TEXT("PeriodicSkillCounter"));

UBTTask_Cat_PeriodicSkillPriority::UBTTask_Cat_PeriodicSkillPriority()
{
	NodeName = TEXT("Cat Periodic Skill Priority");
}

EBTNodeResult::Type UBTTask_Cat_PeriodicSkillPriority::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* /*NodeMemory*/)
{
	UBlackboardComponent* BB = GetBlackboard(OwnerComp);
	if (!BB)
	{
		return EBTNodeResult::Failed;
	}

	// 读数据资产配置（UAISkillDecisionData 基类字段）
	ACharacter* Character = GetOwnerCharacter(OwnerComp);
	UAISkillDecisionData* DecisionData = nullptr;
	if (UCatAIControlData* AIControlData = GetAIControlData(OwnerComp))
	{
		DecisionData = AIControlData->SkillDecisionData;
	}

	if (!DecisionData)
	{
		return EBTNodeResult::Failed;
	}

	const FGameplayTag PeriodicTag = DecisionData->PeriodicSkillTag;
	const int32 Interval = FMath::Max(1, DecisionData->PeriodicSkillInterval);

	// 未启用周期保底：直接不参与，交常规决策
	if (!PeriodicTag.IsValid())
	{
		return EBTNodeResult::Failed;
	}

	// 计数 +1
	const int32 Counter = BB->GetValueAsInt(BBKey_PeriodicSkillCounter) + 1;
	BB->SetValueAsInt(BBKey_PeriodicSkillCounter, Counter);

	// 未满周期：不触发，交常规决策（计数保留累计）
	if (Counter < Interval)
	{
		return EBTNodeResult::Failed;
	}

	// 满周期但目标技能不可激活（次数用尽等）：保留计数，交常规决策，等成功触发才清零
	// 按角色回合组件当前职责（TurnRole）判定，职责由库函数内部查询
	if (!Character || !USVCombatFunctionLibrary::CanActivateTurnAbilityByTagWithCurrentRole(Character, PeriodicTag))
	{
		return EBTNodeResult::Failed;
	}

	// 命中：清零并选中周期技能
	BB->SetValueAsInt(BBKey_PeriodicSkillCounter, 0);
	WriteSelectedAbilityTag(OwnerComp, PeriodicTag);
	return EBTNodeResult::Succeeded;
}

FString UBTTask_Cat_PeriodicSkillPriority::GetStaticDescription() const
{
	return TEXT("周期保底：每 N 次行动强制触发 PeriodicSkillTag（配置于 UAISkillDecisionData）");
}
