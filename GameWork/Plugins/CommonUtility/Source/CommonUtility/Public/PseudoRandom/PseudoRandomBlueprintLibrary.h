// Copyright xiele. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "PseudoRandomBlueprintLibrary.generated.h"

class UPRBPseudoRandomSubsystem;

/**
 * 伪随机算法蓝图函数库
 *
 * 统一外部访问入口，所有伪随机操作通过此库转发到 UPRBPseudoRandomSubsystem（伪随机 PRB 子系统）。
 *
 * 支持两种 Key：
 *   UObject Key  → 传入任意 UObject，自动提取 UniqueID，适合"每角色/每怪物"独立概率
 *   GameplayTag Key → 传入 FGameplayTag，必须属于 Random.Pseudo 层级，适合全局性概率系统
 *
 * 使用方式（Object Key）：
 *   1. PseudoRandomSetExpectedProbability(Self, 0.2)      // 为此角色设 20% 暴击
 *   2. PseudoRandomRoll(Self) → Branch                    // 每次攻击判定
 *      True  → 暴击触发，计数器归零
 *      False → 检查 PseudoRandomGetFailCount(Self)
 *
 * 使用方式（Tag Key）：
 *   1. PseudoRandomSetExpectedProbabilityForTag(Tag, 0.1)  // 设全局事件概率 10%
 *   2. PseudoRandomRollForTag(Tag) → Branch
 */
UCLASS()
class COMMONUTILITY_API UPseudoRandomBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// =============================================================
	//  UObject Key API
	// =============================================================

	/**
	 * 为指定 Object 设置期望概率并重置计数器
	 * @param WorldContextObject 世界上下文
	 * @param Object 目标对象（自动提取 UniqueID 作为 Key）
	 * @param InProbability 期望概率 [0, 1]，如 0.2 = 20%
	 */
	UFUNCTION(BlueprintCallable, Category = "PseudoRandom", meta = (WorldContext = "WorldContextObject"))
	static void PseudoRandomSetExpectedProbability(const UObject* WorldContextObject, UObject* Object, float InProbability);

	/**
	 * 对指定 Object 执行一次概率判定
	 * @return true = 触发，false = 未触发
	 */
	UFUNCTION(BlueprintCallable, Category = "PseudoRandom", meta = (WorldContext = "WorldContextObject"))
	static bool PseudoRandomRoll(const UObject* WorldContextObject, UObject* Object);

	/** 重置指定 Object 的伪随机计数器 */
	UFUNCTION(BlueprintCallable, Category = "PseudoRandom", meta = (WorldContext = "WorldContextObject"))
	static void PseudoRandomReset(const UObject* WorldContextObject, UObject* Object);

	/** 获取指定 Object 当前判定的实际概率 */
	UFUNCTION(BlueprintPure, Category = "PseudoRandom", meta = (WorldContext = "WorldContextObject"))
	static float PseudoRandomGetCurrentProb(const UObject* WorldContextObject, const UObject* Object);

	/** 获取指定 Object 连续未触发次数 */
	UFUNCTION(BlueprintPure, Category = "PseudoRandom", meta = (WorldContext = "WorldContextObject"))
	static int32 PseudoRandomGetFailCount(const UObject* WorldContextObject, const UObject* Object);

	/** 设置指定 Object 的随机种子 */
	UFUNCTION(BlueprintCallable, Category = "PseudoRandom", meta = (WorldContext = "WorldContextObject"))
	static void PseudoRandomSetSeed(const UObject* WorldContextObject, UObject* Object, int32 Seed);

	/** 移除指定 Object 的伪随机数据 */
	UFUNCTION(BlueprintCallable, Category = "PseudoRandom", meta = (WorldContext = "WorldContextObject"))
	static void PseudoRandomRemoveObject(const UObject* WorldContextObject, UObject* Object);

	// =============================================================
	//  GameplayTag Key API（Tag 必须在 Random.Pseudo 层级下）
	// =============================================================

	/** 为指定 Tag 设置期望概率 */
	UFUNCTION(BlueprintCallable, Category = "PseudoRandom|Tag", meta = (WorldContext = "WorldContextObject"))
	static void PseudoRandomSetExpectedProbabilityForTag(const UObject* WorldContextObject, UPARAM(meta = (Categories = "Random.Pseudo")) FGameplayTag Tag, float InProbability);

	/** 对指定 Tag 执行一次判定 */
	UFUNCTION(BlueprintCallable, Category = "PseudoRandom|Tag", meta = (WorldContext = "WorldContextObject"))
	static bool PseudoRandomRollForTag(const UObject* WorldContextObject, UPARAM(meta = (Categories = "Random.Pseudo")) FGameplayTag Tag);

	/** 重置指定 Tag 的伪随机计数器 */
	UFUNCTION(BlueprintCallable, Category = "PseudoRandom|Tag", meta = (WorldContext = "WorldContextObject"))
	static void PseudoRandomResetForTag(const UObject* WorldContextObject, UPARAM(meta = (Categories = "Random.Pseudo")) FGameplayTag Tag);

	/** 获取指定 Tag 当前判定的实际概率 */
	UFUNCTION(BlueprintPure, Category = "PseudoRandom|Tag", meta = (WorldContext = "WorldContextObject"))
	static float PseudoRandomGetCurrentProbForTag(const UObject* WorldContextObject, UPARAM(meta = (Categories = "Random.Pseudo")) FGameplayTag Tag);

	/** 获取指定 Tag 连续未触发次数 */
	UFUNCTION(BlueprintPure, Category = "PseudoRandom|Tag", meta = (WorldContext = "WorldContextObject"))
	static int32 PseudoRandomGetFailCountForTag(const UObject* WorldContextObject, UPARAM(meta = (Categories = "Random.Pseudo")) FGameplayTag Tag);

	/** 设置指定 Tag 的随机种子 */
	UFUNCTION(BlueprintCallable, Category = "PseudoRandom|Tag", meta = (WorldContext = "WorldContextObject"))
	static void PseudoRandomSetSeedForTag(const UObject* WorldContextObject, UPARAM(meta = (Categories = "Random.Pseudo")) FGameplayTag Tag, int32 Seed);

	/** 移除指定 Tag 的伪随机数据 */
	UFUNCTION(BlueprintCallable, Category = "PseudoRandom|Tag", meta = (WorldContext = "WorldContextObject"))
	static void PseudoRandomRemoveTag(const UObject* WorldContextObject, UPARAM(meta = (Categories = "Random.Pseudo")) FGameplayTag Tag);

private:
	static UPRBPseudoRandomSubsystem* GetPseudoRandomSubsystem(const UObject* WorldContextObject);
};
