// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Combat/CatCombatTypes.h"
#include "CatCombatCoreInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UCatCombatCoreInterface : public UInterface
{
	GENERATED_BODY()
};

class CATCOMBATFRAMEWORK_API ICatCombatCoreInterface
{
	GENERATED_BODY()

public:
	/** 死亡开始 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "CatCombatCore")
	void StartDeath();

	/** 死亡结束 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "CatCombatCore")
	void FinishDeath();

	/** 是否死亡 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "CatCombatCore")
	bool IsDeath(bool bDependOnOwner);

	/** 加入战斗队伍 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "CatCombatCore")
	void JoinCombatTeam();

	/** 离开战斗队伍 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "CatCombatCore")
	void LeaveCombatTeam();

	/** 获取该角色身上的主要受击判定范围（被击打方的主要受击反应范围），返回是否成功获取到有效范围 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "CatCombatCore")
	bool GetMainHitReactionRange(FBox& OutHitRange);

	/** 回合开始通知：协调器下发本回合该角色的职责（Attacker 攻击方 / Defender 防守方） */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "CatCombatCore")
	void OnBeginTurn(ECombatTurnRole InRole);
};
