// Copyright xiele. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "PRBPseudoRandomSubsystem.generated.h"

/**
 * PRB 伪随机算法（Pseudo-Random Distribution）状态结构体
 *
 * 存储单个 PRB 伪随机实例的完整运行时状态，作为 TMap 的 Value。
 * 每种概率技能/行为对应一个独立的 FPRBPseudoRandomState。
 */
USTRUCT(BlueprintType)
struct COMMONUTILITY_API FPRBPseudoRandomState
{
	GENERATED_BODY()

	/** 期望概率（如 0.2 = 20%），PRB 伪随机长期期望等于该值。默认 -1 表示未初始化 */
	UPROPERTY(BlueprintReadOnly, Category = "PRBPseudoRandom")
	float ExpectedProbability = -1.0f;

	/** PRB 伪随机初始基础概率 C，自动由期望概率计算得出 */
	UPROPERTY(BlueprintReadOnly, Category = "PRBPseudoRandom")
	float C = 0.055704f;

	/** 当前这次判定的实际概率 */
	UPROPERTY(BlueprintReadOnly, Category = "PRBPseudoRandom")
	float CurrentChance = 0.055704f;

	/** 连续未触发次数 */
	UPROPERTY(BlueprintReadOnly, Category = "PRBPseudoRandom")
	int32 FailCount = 0;

	/**
	 * 保底概率（0~1），默认 0.98
	 * 当 CurrentChance 累积达到此阈值时，强制判定成功，消除极端连空体验。
	 * 设 1.0 等价于关闭保底（靠 PRD 自然保证）。
	 */
	UPROPERTY(BlueprintReadWrite, Category = "PRBPseudoRandom")
	float GuaranteeProbability = 0.98f;

	/** 随机数生成器（内部分配，不序列化） */
	FRandomStream RandomStream;

	/** 重置计数器，概率回退到 C */
	void Reset()
	{
		FailCount = 0;
		CurrentChance = C;
	}
};

/**
 * PRB 伪随机算法（Pseudo-Random Distribution）子系统
 *
 * 核心原理：
 *   每次 Roll 失败后，下次触发概率递增（C × N）；一旦触发成功，概率重置回 C。
 *   长期期望等于期望概率，但短期分布更均匀，消除"连爆/连空"极端体验。
 *
 * 支持两种 Key 类型：
 *   - UObject：自动提取 Object->GetUniqueID() 作为 int32 Key
 *   - FGameplayTag：直接使用 Tag 作为 Key
 *
 * 每种 Key 对应一个独立的 FPRBPseudoRandomState，不同对象/不同 Tag 之间互不干扰。
 */
UCLASS()
class COMMONUTILITY_API UPRBPseudoRandomSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~ End USubsystem Interface

	// =============================================================
	//  UObject Key API（蓝图调用请走 UPseudoRandomBlueprintLibrary）
	//  适用场景：每个角色/怪物的暴击、闪避等独立概率
	// =============================================================

	void SetExpectedProbability(UObject* Object, float InProbability);
	bool Roll(UObject* Object);
	void Reset(UObject* Object);
	float GetCurrentProb(const UObject* Object) const;
	int32 GetFailCount(const UObject* Object) const;
	void SetSeed(UObject* Object, int32 Seed);
	void RemoveObject(UObject* Object);

	/** 设置保底概率（0~1），当累积概率达此阈值时强制触发 */
	UFUNCTION(BlueprintCallable, Category = "PRBPseudoRandom|Object")
	void SetGuaranteeProbability(UObject* Object, float InGuaranteeProb);

	/** 获取当前保底概率 */
	UFUNCTION(BlueprintCallable, Category = "PRBPseudoRandom|Object")
	float GetGuaranteeProbability(const UObject* Object) const;

	// =============================================================
	//  GameplayTag Key API（蓝图调用请走 UPseudoRandomBlueprintLibrary）
	//  约定：传入的 Tag 必须在 Random.Pseudo 层级下（如 Random.Pseudo.Ability.Dodge）
	//  若 Tag 不匹配约定将输出 Warning 并跳过操作
	// =============================================================

	void SetExpectedProbabilityForTag(FGameplayTag Tag, float InProbability);
	bool RollForTag(FGameplayTag Tag);
	void ResetForTag(FGameplayTag Tag);
	float GetCurrentProbForTag(FGameplayTag Tag) const;
	int32 GetFailCountForTag(FGameplayTag Tag) const;
	void SetSeedForTag(FGameplayTag Tag, int32 Seed);
	void RemoveTag(FGameplayTag Tag);

	/** 设置保底概率（0~1），当累积概率达此阈值时强制触发 */
	UFUNCTION(BlueprintCallable, Category = "PRBPseudoRandom|Tag")
	void SetGuaranteeProbabilityForTag(FGameplayTag Tag, float InGuaranteeProb);

	/** 获取当前保底概率 */
	UFUNCTION(BlueprintCallable, Category = "PRBPseudoRandom|Tag")
	float GetGuaranteeProbabilityForTag(FGameplayTag Tag) const;

private:
	// ---- 内部辅助 ----

	/** 二分搜索：计算给定期望概率对应的 PRB 伪随机 C 值 */
	static float ComputeCValue(float ExpectedP);

	/** 核心 Roll 逻辑（操作 FPRBPseudoRandomState） */
	static bool RollInternal(FPRBPseudoRandomState& State);

	/** 从 UObject 提取 UnitID */
	static int32 GetUnitID(const UObject* Object);

	// ---- Object Key ----
	FPRBPseudoRandomState& GetOrCreateObjectState(int32 UnitID);
	const FPRBPseudoRandomState* FindObjectState(int32 UnitID) const;

	// ---- Tag Key ----
	FPRBPseudoRandomState& GetOrCreateTagState(FGameplayTag Tag);
	const FPRBPseudoRandomState* FindTagState(FGameplayTag Tag) const;

	// ---- 数据存储 ----

	/** Object Key → PRB 伪随机状态 */
	TMap<int32, FPRBPseudoRandomState> ObjectPRBPseudoRandomMap;

	/** GameplayTag Key → PRB 伪随机状态 */
	TMap<FGameplayTag, FPRBPseudoRandomState> TagPRBPseudoRandomMap;
};
