// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/UBTTask_CatBase.h"
#include "BTTask_Cat_PeriodicSkillPriority.generated.h"

/**
 * 技能选择：周期保底（通用）——每 N 次行动强制触发一次指定技能。
 *
 * 配置来源：AIControlData->SkillDecisionData（UAISkillDecisionData 基类）：
 *   - PeriodicSkillTag：周期强制技能 Tag（空 = 未启用，直接 Failed）；
 *   - PeriodicSkillInterval：触发周期（默认 4）。
 * 黑板键：PeriodicSkillCounter（Int）累计行动次数；满 N 且目标技能可激活 → 清零并写 SelectedAbilityTag；
 * 满 N 但不可激活 → 计数保留，交下游常规决策（等成功触发才清零）。
 */
UCLASS(Category = "Cat AI", meta = (DisplayName = "Cat Periodic Skill Priority"))
class CATCOMBATFRAMEWORK_API UBTTask_Cat_PeriodicSkillPriority : public UBTTask_CatBase
{
	GENERATED_BODY()

public:
	UBTTask_Cat_PeriodicSkillPriority();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

protected:
	/** 周期保底计数黑板键（Int） */
	static const FName BBKey_PeriodicSkillCounter;
};
