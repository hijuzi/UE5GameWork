// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Combat/SVCombatTypes.h"
#include "SVCombatTurnCoordinator.generated.h"

class ACharacter;
class USVCombatDataStore;
class USVCombatManagerSubsystem;

/**
 * 全局主状态机（回合协调器，决策层）。
 * 职责仅限「阵营级」调度：阵营快照、PendingReadySet 集合收敛、阵营收尾/切换、全灭结算。
 * 不感知角色个体阶段（角色阶段由 USVCharacterTurnComponent 自驱动维护）。
 *
 * 由 USVCombatManagerSubsystem 持有（弱引用记录持有者，避免强引用环）。
 */
UCLASS()
class CATCOMBATFRAMEWORK_API USVCombatTurnCoordinator : public UObject
{
	GENERATED_BODY()

public:
	/** 初始化：记录持有者与数据层（弱引用） */
	void Initialize(USVCombatManagerSubsystem* InOwner, USVCombatDataStore* InDataStore);

	/** 反初始化：清空状态 */
	void Shutdown();

	/** 开始指定阵营的回合：快照该阵营角色 → 下发 Actor/Defender 角色 → 进入 Acting 阶段 */
	void StartCamp(ECombatTeamType TeamType);

	/** 阵营收尾：清理临时状态，切换到对方阵营（由 CampEnd 触发） */
	void EndCamp();

	/** 角色上报"单次行动开始"（由行动 Ability 激活时调用） */
	void NotifyActionStarted(ACharacter* Character);

	/** 角色上报"单次行动结束"（由角色组件 EndAbility 后调用） */
	void NotifyActionFinished(ACharacter* Character);

	/** 角色上报"本回合所有行动完成"：从 PendingReadySet 移除，集合清空则 EndCamp */
	void NotifyTurnFinished(ACharacter* Character);

	/** 中止当前阵营回合（超时/外部中断时调用）：清空待收敛集合并回到 Idle */
	void AbortCurrentCamp();

	/** 重置阵营 StartCamp 计数（进入下一回合时调用） */
	void ResetCampStartCount();

	/** 判断是否所有阵营本轮都已行动过（计数都 ≥ 1） */
	bool AreAllTeamsActed() const;

	/** 判断是否有角色正在 Acting（行动中），供等待其完成（遍历所有战斗角色检查角色级状态） */
	bool IsAnyCharacterActing() const;

	/** 检查并尝试进入「行动收尾（Finishing）」阶段（仍有角色 Acting 时进入/保持等待，返回是否需要继续等待） */
	bool TryEnterFinishing();

	/** 获取指定阵营的对方阵营（纯查询，无副作用） */
	ECombatTeamType GetOpponentTeam(ECombatTeamType TeamType) const;

	/** 获取当前阵营阶段 */
	ECombatCampPhase GetPhase() const { return Phase; }

	/** 获取上一个阵营阶段（调试/观测用，未发生过迁移时为 Idle） */
	ECombatCampPhase GetPreviousPhase() const { return PreviousPhase; }

	/** 获取当前行动阵营 */
	ECombatTeamType GetCurrentTeam() const { return CurrentTeam; }

	/** 获取本回合待收敛的攻击方角色集合（只读，供调试/观测） */
	const TSet<TWeakObjectPtr<ACharacter>>& GetPendingReadySet() const { return PendingReadySet; }

	/** 回合阶段变更事件（对外通知 HUD/相机等表现层） */
	FOnCombatTurnPhaseChanged OnTurnPhaseChanged;

private:
	/** 内部：切换到下一阶段并广播 */
	void SetPhase(ECombatCampPhase NewPhase);

	/** 内部：阵营收尾清理（清空待收敛集合，后续第 6/7 步接入回合临时 Buff 清理） */
	void CleanupCampState();

	/** 内部：进入「行动收尾（Finishing）」阶段（已在 Finishing 阶段则跳过，避免重复广播） */
	void EnterFinishing();

	/** 内部：调度阵营推进（延迟到下一帧调用持有者的 AdvanceCombatTurn，切断同步递归链） */
	void ScheduleAdvanceCombatTurn();

	/** 持有者（弱引用，避免强引用环） */
	UPROPERTY(Transient)
	TWeakObjectPtr<USVCombatManagerSubsystem> OwnerSubsystem;

	/** 数据层（弱引用，由持有者拥有） */
	UPROPERTY(Transient)
	TWeakObjectPtr<USVCombatDataStore> DataStore;

	/** 当前阵营阶段 */
	ECombatCampPhase Phase = ECombatCampPhase::Idle;

	/** 上一个阵营阶段（调试/观测用，SetPhase 迁移前记录） */
	ECombatCampPhase PreviousPhase = ECombatCampPhase::Idle;

	/** 当前行动阵营 */
	ECombatTeamType CurrentTeam = ECombatTeamType::Player;

	/** 本回合待收敛的攻击方角色集合（弱引用；清空即阵营收尾） */
	TSet<TWeakObjectPtr<ACharacter>> PendingReadySet;

	/** 各阵营 StartCamp 计数（用于判断「切换队伍」还是「切换下一回合」） */
	TMap<ECombatTeamType, int32> CampStartCount;
};
