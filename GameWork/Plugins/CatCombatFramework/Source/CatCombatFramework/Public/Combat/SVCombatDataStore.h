// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Combat/SVCombatTypes.h"
#include "SVCombatDataStore.generated.h"

class ACharacter;
class ASVCombatScenePoint;
class USVCombatCharacterDataAsset;
class USVCombatManagerSubsystem;
class UWorld;

/**
 * 战斗数据层。
 * 持有并维护战斗状态数据（角色映射、结算结果），提供数据 CRUD 与查询。
 * 角色增删时通过持有者广播 OnCombatCharacterChanged 事件；不持有 HUDHandler/RoundTimer，仅通过弱引用记录持有者。
 */
UCLASS()
class CATCOMBATFRAMEWORK_API USVCombatDataStore : public UObject
{
	GENERATED_BODY()

public:
	/** 初始化：记录持有者（弱引用） */
	void Initialize(USVCombatManagerSubsystem* InOwner);

	/** 反初始化：清空所有战斗数据 */
	void Shutdown();

	/** 清空所有战斗数据（角色映射 + 结算结果重置） */
	void Reset();

	/** 创建战斗数据：清空旧数据并重置结算结果（开始新战斗时调用） */
	void CreateCombatData(FName InCombatConfigRowName);

	/** 清理战斗数据：清空所有队伍角色映射（结束战斗时调用，保留结算结果） */
	void ClearCombatData(bool bClearTeam);

	// -- 角色数据 CRUD --

	void AddCharacter(ECombatTeamType TeamType, ACharacter* Character);

	/** 移除单个角色，返回是否真的移除 */
	bool RemoveCharacter(ECombatTeamType TeamType, ACharacter* Character);

	/** 清空指定队伍 */
	void ClearTeam(ECombatTeamType TeamType);

	/** 获取指定队伍存活角色列表 */
	TArray<ACharacter*> GetCharacters(ECombatTeamType TeamType) const;

	/** 获取所有队伍的全部存活角色列表 */
	TArray<ACharacter*> GetAllCharacters() const;

	/** 指定队伍是否全员死亡（空队伍视为全灭） */
	bool IsTeamAllDead(ECombatTeamType TeamType) const;

	// -- 结算结果数据 --

	ECombatResultType GetResult() const { return CombatResult; }
	void SetResult(ECombatResultType InResult) { CombatResult = InResult; }

	// -- 战斗状态数据 --

	/** 是否处于战斗中 */
	bool IsInCombat() const { return bInCombat; }
	void SetInCombat(bool bValue) { bInCombat = bValue; }

	/** 获取当前回合所属队伍 */
	ECombatTeamType GetCurrentTurnTeam() const { return CurrentTurnTeam; }

	/** 设置当前回合所属队伍，并广播 OnCombatTeamChanged 事件 */
	void SetCurrentTurnTeam(ECombatTeamType TeamType);

	/** 获取回合开始时的队伍（先手阵营，来自配置，调试/观测用） */
	ECombatTeamType GetStartingTeam() const { return StartingTeam; }

	/** 获取当前回合数（从 1 开始，未开始时为 0） */
	int32 GetRoundNumber() const { return RoundNumber; }

	/** 重置回合数（归零，开始新战斗时调用） */
	void ResetRoundNumber() { RoundNumber = 0; }

	/** 增加回合数（+1，进入下一回合时调用） */
	void IncrementRoundNumber() { ++RoundNumber; }

	/** 特殊设置回合数（仅供特殊场景直接指定，如回放/跳回合） */
	void SetRoundNumber(int32 InRound) { RoundNumber = FMath::Max(0, InRound); }

	/** 推进到下一回合：切回起始阵营（先手阵营），回合数 +1（表示走完一整轮） */
	void AdvanceToNextRound();

	/** 检查战斗是否结束（检测双方是否全灭），返回 true 表示战斗已结束，并写入结算结果 */
	bool CheckCombatEnded();

	/** 获取当前战斗关联的场景点（弱引用，场景点已销毁时返回 nullptr） */
	ASVCombatScenePoint* GetScenePoint() const;

	/** 获取当前战斗的配置行名（调试/观测用，未开始时为 NAME_None） */
	FName GetCombatConfigRowName() const { return CombatConfigRowName; }

	/** 清除所有战斗角色的 Buff 和 ASC 数据 */
	void ClearAllASCData();

	/** 清除所有战斗角色的随从（框架层扩展点，默认空实现） */
	void ClearAllRetinues();

	/** 操控战斗主玩家角色：让玩家控制器 Possess 主战斗角色，并切换主相机（执行层） */
	void PossessMainPlayerCombatCharacter();

private:
	/** 内部：基于队伍全灭状态判断结算结果 */
	ECombatResultType CheckCombatResult() const;

	/** 按配置阵容在场景点站位处生成角色并注册进战斗，返回成功生成数量 */
	int32 SpawnTeamCharacters(UWorld* World, ASVCombatScenePoint* ScenePoint, ECombatTeamType TeamType, const TArray<TObjectPtr<USVCombatCharacterDataAsset>>& Teams);

	/** 持有者（弱引用，避免强引用环） */
	UPROPERTY(Transient)
	TWeakObjectPtr<USVCombatManagerSubsystem> OwnerSubsystem;

	/** 当前战斗关联的场景点（弱引用，不拥有、不阻止 GC；场景点被销毁后自动失效） */
	UPROPERTY(Transient)
	TWeakObjectPtr<ASVCombatScenePoint> CurrentScenePoint;

	/** 战斗角色映射表，按队伍类型分组 */
	UPROPERTY(Transient)
	TMap<ECombatTeamType, FCombatCharacterList> CharacterMap;

	/** 当前战斗结算结果 */
	UPROPERTY(Transient)
	ECombatResultType CombatResult = ECombatResultType::Unsettled;

	/** 是否处于战斗中 */
	UPROPERTY(Transient)
	bool bInCombat = false;

	/** 当前回合所属队伍 */
	UPROPERTY(Transient)
	ECombatTeamType CurrentTurnTeam = ECombatTeamType::Player;

	/** 回合开始时的队伍（先手阵营，来自配置） */
	UPROPERTY(Transient)
	ECombatTeamType StartingTeam = ECombatTeamType::Player;

	/** 当前回合数（从 1 开始，开始新战斗时归零） */
	UPROPERTY(Transient)
	int32 RoundNumber = 0;

	/** 当前战斗的配置行名（调试/观测用，缓存自 CreateCombatData 入参） */
	UPROPERTY(Transient)
	FName CombatConfigRowName = NAME_None;
};
