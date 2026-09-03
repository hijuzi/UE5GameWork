// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UObject/WeakObjectPtr.h"
#include "Combat/SVCombatTypes.h"
#include "SVCombatManagerSubsystem.generated.h"

class ACharacter;
class ASVCombatScenePoint;
class USVCombatDataStore;
class USVCombatHUDHandler;
class USVCombatRoundTimer;
class USVCombatTurnCoordinator;

/** 战斗队伍切换委托 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatTeamChanged, ECombatTeamType, TeamType);

/** 战斗角色加入/离开委托 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCombatCharacterChanged, bool, bJoined, ECombatTeamType, TeamType, ACharacter*, Character);

/** 战斗结算完成委托（扩展点：外部剧情/流程系统绑定接入结算后续逻辑） */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatSettled, ECombatResultType, Result);

/**
 * 战斗系统管理器（GameInstanceSubsystem）
 * 生命周期与 GameInstance 绑定，负责管理战斗流程、回合逻辑、战斗状态等核心战斗功能。
 * 数据状态（角色映射、结算结果）由数据层 USVCombatDataStore 维护。
 */
UCLASS(BlueprintType, Blueprintable)
class CATCOMBATFRAMEWORK_API USVCombatManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** 决定是否创建此 Subsystem，默认返回 true */
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	/** Subsystem 初始化，在所有 Subsystem 注册完成后调用 */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Subsystem 反初始化，GameInstance 销毁时调用 */
	virtual void Deinitialize() override;

	void StartBattle(FName CombatConfigRowName = NAME_None);

	bool EndBattle(bool bClearTeam = false);

	/** 推进到战斗的下一个完整回合：切回起始阵营 + 回合数 +1 + 重置计时，并开始起始阵营的回合（不区分切队伍） */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void AdvanceToNextRound();

	/** 推进战斗回合：一个阵营结束后，根据计数判断「切换队伍（切对方阵营）」还是「切换下一回合（调 AdvanceToNextRound）」并执行 */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void AdvanceCombatTurn();

	/** 获取战斗数据层 */
	USVCombatDataStore* GetDataStore() const { return DataStore; }

	/** 获取战斗 HUD 处理器（执行层） */
	USVCombatHUDHandler* GetHUDHandler() const { return HUDHandler; }

	/** 获取回合计时器（执行层） */
	USVCombatRoundTimer* GetRoundTimer() const { return RoundTimer; }

	/** 获取回合协调器（决策层，全局主状态机） */
	USVCombatTurnCoordinator* GetTurnCoordinator() const { return TurnCoordinator; }

	/** 战斗队伍切换事件 */
	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnCombatTeamChanged OnCombatTeamChanged;

	/** 战斗角色加入/离开事件 */
	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnCombatCharacterChanged OnCombatCharacterChanged;

	/** 战斗结算完成事件（SettleCombat 后广播，扩展点） */
	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnCombatSettled OnCombatSettled;

	// -- 战斗参与者管理 --

	/** 获取当前战斗结算结果（转发数据层） */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat")
	ECombatResultType GetCombatResult() const;

	/** 战斗结算处理 */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SettleCombat();

	/** 通过队伍类型获取角色列表 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat")
	TArray<ACharacter*> GetCombatCharacterList(ECombatTeamType TeamType) const;

	/** 获取当前回合进度，返回 [0, 1] */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat|Round")
	float GetRoundProgress() const;

	/** 尝试执行角色加入/离开战斗的 Ability（根据 bJoined 选择 Join/Leave 标签，执行层，由 HandleCombatCharacterChanged 触发） */
	void TryExecuteCombatCharacterAbility(bool bJoined, ACharacter* Character);

	// -- 场景点注册表 --
	/** 注册场景点（键为配置行名；行名重复且非同一对象时输出警告并覆盖注册） */
	void RegisterScenePoint(ASVCombatScenePoint* ScenePoint);

	/** 注销场景点（按配置行名移除，附指针校验避免误删） */
	void UnregisterScenePoint(ASVCombatScenePoint* ScenePoint);

	/** 按配置行名获取已注册场景点（未注册或已销毁返回 nullptr） */
	ASVCombatScenePoint* GetScenePoint(FName RowName) const;

protected:
	/** 战斗数据层（纯数据 CRUD） */
	UPROPERTY(Transient)
	TObjectPtr<USVCombatDataStore> DataStore;

	/** 战斗 HUD 处理器（执行层） */
	UPROPERTY(Transient)
	TObjectPtr<USVCombatHUDHandler> HUDHandler;

	/** 回合计时器（执行层） */
	UPROPERTY(Transient)
	TObjectPtr<USVCombatRoundTimer> RoundTimer;

	/** 回合协调器（决策层，全局主状态机） */
	UPROPERTY(Transient)
	TObjectPtr<USVCombatTurnCoordinator> TurnCoordinator;

private:
	/** 战斗角色加入/离开时联动输入映射（执行层，由 OnCombatCharacterChanged 触发） */
	UFUNCTION()
	void HandleCombatCharacterChanged(bool bJoined, ECombatTeamType TeamType, ACharacter* Character);

	/** 回合计时超时处理（决策层：停止当前任务） */
	void HandleRoundTimeExpired();

	/** 场景点注册表：键为配置行名，值为场景点弱引用（不拥有、不阻止 GC） */
	TMap<FName, TWeakObjectPtr<ASVCombatScenePoint>> ScenePointMap;
};
