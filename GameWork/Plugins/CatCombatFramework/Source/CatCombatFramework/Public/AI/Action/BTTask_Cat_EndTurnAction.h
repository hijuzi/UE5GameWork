// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Cat_EndTurnAction.generated.h"

/**
 * 结束回合行动 Task：检查黑板 SelectedAbilityTag 当前能否激活（按当前职责）。
 * - 未选择技能，或该技能当前不可激活（次数用尽/冷却/职责不符等）：经战斗管理器 USVCombatManagerSubsystem::AdvanceCombatTurn 推进战斗回合（切队伍/下一回合）；
 * - 技能可激活：返回 Failed，不结束回合，交由后续激活流程继续执行。
 */
UCLASS(Category = "Cat AI", meta = (DisplayName = "Cat End Turn Action"))
class CATCOMBATFRAMEWORK_API UBTTask_Cat_EndTurnAction : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_Cat_EndTurnAction();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
