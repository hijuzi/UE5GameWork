// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SVCombatTypes.generated.h"

class ACharacter;
class UGameplayAbility;

/** 战斗队伍类型 */
UENUM(BlueprintType)
enum class ECombatTeamType : uint8
{
	Player     UMETA(DisplayName = "玩家队伍"),
	Enemy      UMETA(DisplayName = "敌人队伍"),
};

/** 战斗结算结果 */
UENUM(BlueprintType)
enum class ECombatResultType : uint8
{
	Unsettled   UMETA(DisplayName = "未结束"),
	PlayerWin   UMETA(DisplayName = "玩家获胜"),
	EnemyWin    UMETA(DisplayName = "敌人获胜"),
};

/** 战斗角色列表容器，用于 TMap 值类型（UHT 不允许 TArray 直接作为 TMap 值） */
USTRUCT()
struct FCombatCharacterList
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TWeakObjectPtr<ACharacter>> Characters;
};

// ============================================================================
// 回合制相关类型（原 SVCombatTurnTypes.h，已合并至此）
// ============================================================================

/**
 * 阵营回合阶段（全局主状态机 USVCombatTurnCoordinator 的状态）。
 * 仅描述"阵营级"流转，不持有角色个体阶段。
 */
UENUM(BlueprintType)
enum class ECombatCampPhase : uint8
{
	Idle             UMETA(DisplayName = "空闲"),
	Starting         UMETA(DisplayName = "阵营开始"),
	Acting           UMETA(DisplayName = "行动中"),
	Finishing        UMETA(DisplayName = "行动Finishing"),
	CampEnd          UMETA(DisplayName = "阵营收尾"),
	Settle           UMETA(DisplayName = "结算"),
};

/**
 * 角色自身回合阶段（角色级状态机 USVCharacterTurnComponent 的状态）。
 */
UENUM(BlueprintType)
enum class ECharacterTurnState : uint8
{
	Idle        UMETA(DisplayName = "空闲"),
	Selecting   UMETA(DisplayName = "选择中"),
	Acting      UMETA(DisplayName = "行动中"),
	Defending   UMETA(DisplayName = "防守中"),
	Deferred    UMETA(DisplayName = "挂起"),
	Acted       UMETA(DisplayName = "已行动"),
};

/**
 * 角色在本回合的职责。
 * 由「自身阵营 == 协调器 CurrentTeam」判定，在 CampStarted 时一次性下发并保持稳定。
 */
UENUM(BlueprintType)
enum class ECombatTurnRole : uint8
{
	None     UMETA(DisplayName = "无"),
	Attacker UMETA(DisplayName = "攻击方（主动出手）"),
	Defender UMETA(DisplayName = "防守方（被动响应）"),
};

/**
 * 单次行动请求。
 * 玩家输入桥接与敌人 AI 决策统一产出此结构，由 SVCombatActionRequester 激活对应 Ability。
 */
USTRUCT(BlueprintType)
struct FActionRequest
{
	GENERATED_BODY()

	/** 技能 AbilityTag（用于定位要激活的 Ability） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Turn")
	FGameplayTag AbilityTag;

	/** 行动目标（由输入/AI 桥接写入角色 AttackTarget 后，能力通过 ICombatInterface 读取；此处仅作请求声明用途） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Turn")
	TWeakObjectPtr<AActor> Target;
};

/**
 * 跨回合待办行动（蓄力/延迟/持续 N 回合）。
 * 主状态机在 CampEnd 广播回合结束后，角色组件自减 RemainingTurns，归零后于下一回合激活。
 */
USTRUCT(BlueprintType)
struct FPendingTurnAction
{
	GENERATED_BODY()

	/** 待激活的 Ability */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Turn")
	TSubclassOf<UGameplayAbility> AbilityClass;

	/** 技能 AbilityTag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Turn")
	FGameplayTag AbilityTag;

	/** 行动目标 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Turn")
	TWeakObjectPtr<AActor> Target;

	/** 剩余回合数，每过一回合 -1，归零触发 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Turn", meta = (ClampMin = "0"))
	int32 RemainingTurns = 0;
};

/** 实时防御请求（防守方非己方回合全程可主动触发） */
USTRUCT(BlueprintType)
struct FReactiveDefenseRequest
{
	GENERATED_BODY()

	/** 防御类型（如闪避/格挡/弹反，通过「类型 → 处理节点」注册扩展） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Defend")
	FGameplayTag DefenseType;

	/** 攻击来源 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Defend")
	TWeakObjectPtr<AActor> AttackSource;
};

/** 实时防御结果 */
USTRUCT(BlueprintType)
struct FReactiveDefenseResult
{
	GENERATED_BODY()

	/** 防御是否成功 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Defend")
	bool bSucceeded = false;
};

/** 受击结算上下文（受击命中结算这一抽象时刻的钩子入参） */
USTRUCT(BlueprintType)
struct FHitResolveContext
{
	GENERATED_BODY()

	/** 攻击来源 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Defend")
	TWeakObjectPtr<AActor> AttackSource;

	/** 是否命中 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Defend")
	bool bHit = false;

	/** 命中结果标签（如 Ability.Combat.Defend.Dodge/Block/Counter） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Defend")
	FGameplayTag HitResultTag;
};

// -- 内部推进委托（强耦合，可断点，主状态机与角色组件/能力基类直接互调） --

/** 阵营回合开始（广播给该阵营角色组件，携带其角色职责） */
DECLARE_DELEGATE_TwoParams(FOnCombatCampTurnStarted, ECombatTeamType /*TeamType*/, ECombatTurnRole /*Role*/);

/** 回合阶段变更 */
DECLARE_DELEGATE_TwoParams(FOnCombatTurnPhaseChanged, ECombatCampPhase /*Phase*/, ECombatTeamType /*TeamType*/);

/** 单角色行动结束（角色组件上报，供协调器收敛 PendingReadySet） */
DECLARE_DELEGATE_OneParam(FOnCombatTurnActionFinished, ACharacter* /*Character*/);
