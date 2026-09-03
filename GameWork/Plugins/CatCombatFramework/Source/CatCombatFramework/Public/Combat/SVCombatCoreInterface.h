// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Combat/SVCombatTypes.h"
#include "SVCombatCoreInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class USVCombatCoreInterface : public UInterface
{
	GENERATED_BODY()
};

class CATCOMBATFRAMEWORK_API ISVCombatCoreInterface
{
	GENERATED_BODY()

public:
	/** 死亡开始 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "SVCombatCore")
	void StartDeath();

	/** 死亡结束 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "SVCombatCore")
	void FinishDeath();

	/** 是否死亡 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "SVCombatCore")
	bool IsDeath(bool bDependOnOwner);

	/** 加入战斗队伍 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "SVCombatCore")
	void JoinCombatTeam();

	/** 离开战斗队伍 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "SVCombatCore")
	void LeaveCombatTeam();

	/** 回合开始通知：协调器下发本回合该角色的职责（Attacker 攻击方 / Defender 防守方） */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "SVCombatCore")
	void OnBeginTurn(ECombatTurnRole InRole);
};
