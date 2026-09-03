// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SVCombatEnemyInterface.generated.h"

class AActor;

/**
 * 战斗敌人接口（扩展点）。
 *
 * 替代源项目对 ASVEnemyBase / ICombatInterface 的强依赖：
 * 敌人决策桥接（USVCombatEnemyDecisionBridge）通过此接口获取敌人的攻击目标等状态。
 * 具体敌人角色类实现此接口以接入 AI 决策。
 */
UINTERFACE(MinimalAPI, Blueprintable)
class USVCombatEnemyInterface : public UInterface
{
	GENERATED_BODY()
};

class CATCOMBATFRAMEWORK_API ISVCombatEnemyInterface
{
	GENERATED_BODY()

public:
	/** 获取敌人当前攻击目标（由具体敌人实现） */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|AI")
	AActor* GetAttackTargetActor() const;
};
