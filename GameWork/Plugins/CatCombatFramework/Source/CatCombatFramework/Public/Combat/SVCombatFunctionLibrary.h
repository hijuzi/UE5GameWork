// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Combat/SVCombatHUDHandler.h"
#include "Combat/SVCombatTypes.h"
#include "SVCombatFunctionLibrary.generated.h"

class ACharacter;
struct FSVCombatDataTableRow;

/**
 * 战斗蓝图函数库：提供开战/结束/查询/行动请求等蓝图可调用入口。
 */
UCLASS()
class CATCOMBATFRAMEWORK_API USVCombatFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	static bool StopPlayerAction();

	/** 试图开始战斗：按配置行名查找对应场景点，按其配置生成双方阵容角色后开启战斗 */
	UFUNCTION(BlueprintCallable, Category = "Combat", meta = (WorldContext = "WorldContextObject"))
	static bool TryStartBattle(const UObject* WorldContextObject,
		UPARAM(meta = (GetOptions = "GetCombatConfigRowNames")) FName CombatConfigRowName);

	/** 供蓝图节点参数下拉框使用的选项：直接复用 ASVCombatScenePoint 的静态实现 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat")
	static TArray<FName> GetCombatConfigRowNames();

	/** 试图结束战斗 */
	UFUNCTION(BlueprintCallable, Category = "Combat", meta = (WorldContext = "WorldContextObject"))
	static bool TryEndBattle(const UObject* WorldContextObject, bool bClearTeam = false);

	/** 是否处于战斗中 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat", meta = (WorldContext = "WorldContextObject"))
	static bool IsInCombat(const UObject* WorldContextObject);

	/** 设置战斗 HUD 可见性 */
	UFUNCTION(BlueprintCallable, Category = "Combat", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "Params"))
	static void SetBattleHUDVisible(const UObject* WorldContextObject, bool bVisible, const FSVCombatHUDVisibilityParams& Params = FSVCombatHUDVisibilityParams());

	/** 设置战斗回合暂停 */
	UFUNCTION(BlueprintCallable, Category = "Combat", meta = (WorldContext = "WorldContextObject"))
	static void SetCombatRoundPaused(const UObject* WorldContextObject, bool bPaused);

	/** 添加战斗角色到指定队伍 */
	UFUNCTION(BlueprintCallable, Category = "Combat", meta = (WorldContext = "WorldContextObject"))
	static void AddCombatCharacter(const UObject* WorldContextObject, ECombatTeamType TeamType, ACharacter* Character);

	/** 从指定队伍中移除单个角色 */
	UFUNCTION(BlueprintCallable, Category = "Combat", meta = (WorldContext = "WorldContextObject"))
	static void RemoveCombatCharacter(const UObject* WorldContextObject, ECombatTeamType TeamType, ACharacter* Character);

	/** 获取指定队伍的战斗角色列表 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat", meta = (WorldContext = "WorldContextObject"))
	static TArray<ACharacter*> GetCombatCharacterList(const UObject* WorldContextObject, ECombatTeamType TeamType);

	/** 获取玩家主战斗角色（玩家队伍中第一个角色） */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat", meta = (WorldContext = "WorldContextObject"))
	static ACharacter* GetMainPlayerCombatCharacter(const UObject* WorldContextObject);

	/** 获取敌方主战斗角色（敌方队伍中第一个角色） */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat", meta = (WorldContext = "WorldContextObject"))
	static ACharacter* GetMainEnemyCombatCharacter(const UObject* WorldContextObject);

	/** 按行动请求激活 Ability（统一玩家/AI 激活入口，按 AbilityTag 触发），返回是否成功触发激活 */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	static bool RequestAction(ACharacter* Actor, const FActionRequest& Request);

	/** 按配置行名从当前世界中查找对应场景点，并获取其战斗配置数据表行（任一级查找失败返回 nullptr） */
	static const FSVCombatDataTableRow* GetCombatConfigRowFromWorld(const UObject* WorldContextObject, FName CombatConfigRowName);

private:
	static class USVCombatManagerSubsystem* GetCombatManagerSubsystem(const UObject* WorldContextObject);
};
