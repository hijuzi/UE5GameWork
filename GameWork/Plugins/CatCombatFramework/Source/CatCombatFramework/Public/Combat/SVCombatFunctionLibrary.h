// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AI/CatAISkillDecisionData.h"
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

	// -- AI 辅助（行为树决策） --

	/** 获取指定角色所属队伍（从数据层反查角色映射表），返回是否找到；找到时通过 OutTeamType 输出其队伍 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat|AI")
	static bool GetCharacterTeam(ACharacter* Character, ECombatTeamType& OutTeamType);

	/** 获取指定阵营的对方阵营（纯查询，无副作用） */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat")
	static ECombatTeamType GetOpponentTeam(ECombatTeamType TeamType);

	/** 获取指定角色的敌方阵营角色列表：反查其所属队伍后取对方阵营；链路任一无效返回空列表 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat|AI")
	static TArray<ACharacter*> GetOpponentCombatCharacterList(ACharacter* Character);

	/** 获取指定角色所在阵营的敌方主战斗角色（敌方阵营列表首个角色）；链路无效返回 nullptr */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat|AI")
	static ACharacter* GetMainOpponentCombatCharacter(ACharacter* Character);

	/** 按行动请求激活 Ability（统一玩家/AI 激活入口，按 AbilityTag 触发），返回是否成功触发激活 */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	static bool RequestAction(ACharacter* Actor, const FActionRequest& Request);

	/** 判断指定角色当前是否可激活指定 Tag 的回合能力（按职责查询角色回合组件，调用 ASC 原生 CanActivateAbility） */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat")
	static bool CanActivateTurnAbilityByTag(ACharacter* Character, const FGameplayTag& Tag, ECombatTurnRole Role);

	/** 判断指定角色当前是否可激活指定 Tag 的回合能力（攻击方或防守方任意职责可激活即返回 true） */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat")
	static bool CanActivateTurnAbilityByTagAnyRole(ACharacter* Character, const FGameplayTag& Tag);

	/** 判断指定角色当前是否可激活指定 Tag 的回合能力（按角色回合组件的当前职责 TurnRole 查询，供「当前攻/防」场景直接调用） */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat")
	static bool CanActivateTurnAbilityByTagWithCurrentRole(ACharacter* Character, const FGameplayTag& Tag);

	/** 判断技能 Tag 是否属于「空技能」家族（Ability.TurnAction.EmptyAction 本身或其子 Tag），用于跳过激活校验 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat|AI")
	static bool IsEmptyActionTag(const FGameplayTag& SkillTag);

	/**
	 * 从技能权重表过滤出候选：要求 SkillTag 有效、RequiredStateTag 命中（留空=不限制状态；否则需角色当前持有该状态 Tag，如 Status.Health.Low）、且当前职责可激活。
	 * 成功后通过 OutTag 输出唯一选中的技能 Tag，返回 true。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat|AI")
	static bool PickSkillFromWeightTable(ACharacter* Character, ECombatTurnRole Role,
		const TArray<FAISkillWeight>& SkillTable, FGameplayTag& OutTag);

	/** 按配置行名从当前世界中查找对应场景点，并获取其战斗配置数据表行（任一级查找失败返回 nullptr） */
	static const FSVCombatDataTableRow* GetCombatConfigRowFromWorld(const UObject* WorldContextObject, FName CombatConfigRowName);

private:
	static class USVCombatManagerSubsystem* GetCombatManagerSubsystem(const UObject* WorldContextObject);

	/** 从源权重表过滤出「角色持有 RequiredStateTag（或未指定状态）+ 当前职责可激活」的候选条目 */
	static void CollectSkillCandidates(const TArray<FAISkillWeight>& Source, ACharacter* Character,
		class USVCharacterTurnComponent* TurnComp, ECombatTurnRole Role, TArray<FAISkillWeight>& OutCandidates);

	/** 对候选列表做加权随机，返回是否成功并输出选中 Tag */
	static bool PickWeightedSkill(const TArray<FAISkillWeight>& Candidates, FGameplayTag& OutTag);
};
