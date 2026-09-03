// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "Combat/SVCombatTypes.h"
#include "SVCombatEnemyDecisionBridge.generated.h"

class ACharacter;

/**
 * 攻击技能权重配置项（框架内定义，替代源项目对 StateTree 任务 UEnemyAttackTask 的 FEnemyAttackTagWeight 依赖）。
 */
USTRUCT(BlueprintType)
struct FEnemyAttackTagWeight
{
	GENERATED_BODY()

	/** 攻击技能标签 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|AI")
	FGameplayTag AttackTag;

	/** 权重（越大越容易被选中） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|AI", meta = (ClampMin = "0"))
	int32 Weight = 1;
};

/**
 * 敌人 AI 决策桥接（适配层）。
 * 重新实现「按权重选攻击标签」决策逻辑（已释放池 + 冷却过滤 + 权重随机），
 * 把决策结果包装成 FActionRequest，交由 USVCombatFunctionLibrary::RequestAction 统一激活。
 *
 * 移植说明：敌人类型抽象为 ACharacter，攻击目标经 ISVCombatEnemyInterface 获取；
 * 已释放技能池由桥接层内部维护（替代源项目 Enemy->ReleasedAttackTags）。
 */
UCLASS()
class CATCOMBATFRAMEWORK_API USVCombatEnemyDecisionBridge : public UObject
{
	GENERATED_BODY()

public:
	/** 为指定敌人产出一个行动请求（选技能 tag + 目标），返回是否成功产出 */
	UFUNCTION(BlueprintCallable, Category = "Combat|AI")
	bool RequestEnemyAction(ACharacter* Enemy, FActionRequest& OutRequest);

	/** 攻击技能权重配置 */
	UPROPERTY(EditAnywhere, Category = "Combat|AI", DisplayName = "攻击技能权重")
	TArray<FEnemyAttackTagWeight> AttackTagWeights;

private:
	/** 按权重从敌人技能池选一个攻击标签（已释放池 + 冷却过滤 + 权重随机） */
	FGameplayTag SelectAttackTag(ACharacter* Enemy);

	/** 判断标签对应的技能是否处于冷却中 */
	bool IsAttackTagOnCooldown(const FGameplayTag& AttackTag, ACharacter* Enemy) const;

	/** 每个敌人的已释放技能池（桥接层内部维护） */
	TMap<TWeakObjectPtr<ACharacter>, TArray<FGameplayTag>> ReleasedPools;
};
