// Copyright xiele. All Rights Reserved.

#include "PseudoRandom/PRBPseudoRandomSubsystem.h"
#include "PseudoRandom/PseudoRandomLog.h"
#include "Engine/GameInstance.h"

/**
 * PRB 伪随机算法（Pseudo-Random Distribution）子系统 实现
 *
 * 整体架构：
 *   1. 两个 TMap 分别以 int32(UnitID) 和 FGameplayTag 为 Key，存储 FPRBPseudoRandomState
 *   2. 首次访问时自动创建状态（懒初始化），RandomStream 自动生成随机种子
 *   3. 核心算法在 RollInternal() 中：失败则概率递增 C×(FailCount+1)，成功则重置
 *   4. C 值由 ComputeCValue() 二分搜索精确求解，保证长期期望 == ExpectedProbability
 */

// =============================================================
//  生命周期
// =============================================================

void UPRBPseudoRandomSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogPseudoRandom, Log, TEXT("[PRBPseudoRandomSubsystem] Initialized - TMap-based multi-instance PRBPseudoRandom"));
}

void UPRBPseudoRandomSubsystem::Deinitialize()
{
	// 清空所有 PRB 伪随机状态，释放内存
	ObjectPRBPseudoRandomMap.Empty();
	TagPRBPseudoRandomMap.Empty();
	UE_LOG(LogPseudoRandom, Log, TEXT("[PRBPseudoRandomSubsystem] Deinitialized - all PRBPseudoRandom states cleared"));
}

// =============================================================
//  内部辅助
// =============================================================

int32 UPRBPseudoRandomSubsystem::GetUnitID(const UObject* Object)
{
	// 提取 Object 的唯一 ID 作为 TMap Key
	// GetUniqueID() 在同一 GameInstance 生命周期内稳定不变
	if (!Object)
	{
		return INDEX_NONE;
	}
	return static_cast<int32>(Object->GetUniqueID());
}

// -------------------------------------------------------------------
//  RollInternal：核心判定逻辑
//
//  执行一次 PRB 伪随机判定，操作传入的 State：
//    - 生成 [0, 1) 随机数，与 State.CurrentChance 比较
//    - 小于 CurrentChance → 触发成功，调用 Reset() 归零计数器
//    - 大于等于 CurrentChance → 触发失败，FailCount++，下次概率递增
//
//  概率递增公式：
//    P_next = min(C × (FailCount + 1), 1.0)
//    即第 1 次失败后概率 = C×2，第 2 次 = C×3，以此类推
// -------------------------------------------------------------------
bool UPRBPseudoRandomSubsystem::RollInternal(FPRBPseudoRandomState& State)
{
	// 保底机制：当累积概率达到保底阈值时，强制判定成功，消除极端连空体验
	// 例如 GuaranteeProbability=0.98 → 最多连续失败 ceil(0.98/C)-1 次后必定触发
	if (State.CurrentChance >= State.GuaranteeProbability)
	{
		State.Reset();
		return true;
	}

	// 生成 [0, 1) 随机数
	const float RollValue = State.RandomStream.FRand();

	if (RollValue < State.CurrentChance)
	{
		// 触发成功 → 计数器归零，概率回到 C
		State.Reset();
		return true;
	}

	// 触发失败 → 递增连续失败计数，更新下次判定的概率
	++State.FailCount;
	State.CurrentChance = FMath::Min(State.C * (State.FailCount + 1), 1.0f);

	// 保底兜底：更新后的 CurrentChance 若已达保底阈值，下次 Roll 必定触发
	return false;
}

// -------------------------------------------------------------------
//  ComputeCValue：二分搜索计算 PRB 伪随机算法的 C 值
//
//  问题描述：
//    给定期望概率 ExpectedP（如 0.2 = 20%），求 C 使得每次判定的长期
//    期望触发概率 == ExpectedP。
//
//    P(first hit on trial n) = P_n × Π(i=1..n-1)(1 - P_i)
//    其中 P_n = min(n × C, 1.0)
//
//    Expected trials = E = Σ(n=1..∞) n × P(first hit on trial n)
//    Per-trial probability = 1 / E
//
//    求解：|1/E - ExpectedP| < Tolerance
//
//  解法：
//    因 1/E 关于 C 单调递增，使用二分搜索在 [0, ExpectedP] 内求解。
//    C 取值范围约在 [ExpectedP²/2, ExpectedP]，Upper bound 设为 ExpectedP 是安全的。
//
//  示例（标准 PRD 常量）：
//    ExpectedP=0.20 → C≈0.05570 (Dota2 PRD: 5.570%)
//    ExpectedP=0.10 → C≈0.02020
//    ExpectedP=0.50 → C≈0.21830
//    ExpectedP=0.01 → C≈0.00015
// -------------------------------------------------------------------
float UPRBPseudoRandomSubsystem::ComputeCValue(float ExpectedP)
{
	// 边界情况
	if (ExpectedP <= 0.0f)
	{
		return 0.0f;
	}
	if (ExpectedP >= 1.0f)
	{
		return 1.0f;
	}

	// 二分搜索：C ∈ (0, ExpectedP]，最多 50 次迭代
	float Low = 0.0f;
	float High = ExpectedP;
	const int32 MaxIterations = 50;
	const float Tolerance = 1e-7f;

	for (int32 Iter = 0; Iter < MaxIterations; ++Iter)
	{
		const float Mid = (Low + High) * 0.5f;

		// ---- 内层：计算期望判定次数 E ----
		// E = Σ n × P(第 n 次首次命中)
		// 不设硬上限：持续累加直到 Pn==1.0 或 FailProduct 收敛到可忽略
		float E = 0.0f;                   // Expected trials to first hit
		float FailProduct = 1.0f;          // Π(1 - P_i), 前 n-1 次全失败的概率

		for (int32 n = 1; ; ++n)
		{
			// 第 n 次判定的概率 P_n = n × C，上限 1.0
			const float Pn = FMath::Min(static_cast<float>(n) * Mid, 1.0f);

			// n × P(第 n 次首次命中)
			E += static_cast<float>(n) * Pn * FailProduct;

			// P_n == 1.0 → 必然触发，终止
			if (Pn >= 1.0f)
			{
				break;
			}

			FailProduct *= (1.0f - Pn);

			// FailProduct 已经小到可忽略 → 后续项对 E 贡献极微，终止
			if (FailProduct < 1e-10f)
			{
				break;
			}
		}

		// per-trial 概率 = 1 / E（E > 0 始终成立因为 C > 0）
		const float PerTrialP = 1.0f / E;

		// ---- 二分搜索收敛判断 ----
		if (FMath::Abs(PerTrialP - ExpectedP) < Tolerance)
		{
			return Mid;
		}

		if (PerTrialP < ExpectedP)
		{
			Low = Mid;                      // C 太小，需要增大
		}
		else
		{
			High = Mid;                     // C 太大，需要减小
		}
	}

	// 返回二分结果的中间值
	return (Low + High) * 0.5f;
}

// -------------------------------------------------------------------
//  IsValidPseudoRandomTag：校验 Tag 必须在 Random.Pseudo 层级下
//  所有 Tag API 入口均调用此函数，不匹配的 Tag 将输出 Warning 并跳过操作
// -------------------------------------------------------------------
static bool IsValidPseudoRandomTag(FGameplayTag Tag)
{
	static const FGameplayTag RootTag = FGameplayTag::RequestGameplayTag(TEXT("Random.Pseudo"), false);
	return Tag.MatchesTag(RootTag);
}

// =============================================================
//  Object Key 内部存取
//  采用懒初始化模式：首次访问时自动创建 FPRBPseudoRandomState
// =============================================================

FPRBPseudoRandomState& UPRBPseudoRandomSubsystem::GetOrCreateObjectState(int32 UnitID)
{
	// 已有状态 → 直接返回引用
	FPRBPseudoRandomState* Found = ObjectPRBPseudoRandomMap.Find(UnitID);
	if (Found)
	{
		return *Found;
	}

	// 首次访问 → 创建新状态并初始化随机种子
	FPRBPseudoRandomState NewState;
	NewState.RandomStream.GenerateNewSeed();    // 使用系统时间生成种子，保证各实例独立
	return ObjectPRBPseudoRandomMap.Add(UnitID, NewState);
}

const FPRBPseudoRandomState* UPRBPseudoRandomSubsystem::FindObjectState(int32 UnitID) const
{
	return ObjectPRBPseudoRandomMap.Find(UnitID);
}

// =============================================================
//  Tag Key 内部存取
// =============================================================

FPRBPseudoRandomState& UPRBPseudoRandomSubsystem::GetOrCreateTagState(FGameplayTag Tag)
{
	FPRBPseudoRandomState* Found = TagPRBPseudoRandomMap.Find(Tag);
	if (Found)
	{
		return *Found;
	}

	FPRBPseudoRandomState NewState;
	NewState.RandomStream.GenerateNewSeed();
	return TagPRBPseudoRandomMap.Add(Tag, NewState);
}

const FPRBPseudoRandomState* UPRBPseudoRandomSubsystem::FindTagState(FGameplayTag Tag) const
{
	return TagPRBPseudoRandomMap.Find(Tag);
}

// =============================================================
//  UObject Key API 实现
//  外部通过 UPseudoRandomBlueprintLibrary 调用，此处为纯 C++ 接口
// =============================================================

void UPRBPseudoRandomSubsystem::SetExpectedProbability(UObject* Object, float InProbability)
{
	if (!Object)
	{
		UE_LOG(LogPseudoRandom, Warning, TEXT("[PRBPseudoRandomSubsystem] SetExpectedProbability: Object is null"));
		return;
	}

	// Clamp 到合法范围
	InProbability = FMath::Clamp(InProbability, 0.0f, 1.0f);

	const int32 UnitID = GetUnitID(Object);
	FPRBPseudoRandomState& State = GetOrCreateObjectState(UnitID);

	// 概率未变化则跳过，避免重复计算 C 值
	if (FMath::IsNearlyEqual(State.ExpectedProbability, InProbability))
	{
		return;
	}

	State.ExpectedProbability = InProbability;
	State.C = ComputeCValue(InProbability);
	State.Reset();                              // 改概率时重置计数器，避免旧状态干扰
}

bool UPRBPseudoRandomSubsystem::Roll(UObject* Object)
{
	if (!Object)
	{
		UE_LOG(LogPseudoRandom, Warning, TEXT("[PRBPseudoRandomSubsystem] Roll: Object is null"));
		return false;
	}

	// 未设概率时默认使用 20%（FPRBPseudoRandomState 默认值），C 保持初始值无需额外计算
	FPRBPseudoRandomState& State = GetOrCreateObjectState(GetUnitID(Object));
	return RollInternal(State);
}

void UPRBPseudoRandomSubsystem::Reset(UObject* Object)
{
	if (!Object)
	{
		return;
	}

	const int32 UnitID = GetUnitID(Object);
	if (FPRBPseudoRandomState* State = ObjectPRBPseudoRandomMap.Find(UnitID))
	{
		State->Reset();                         // FailCount=0, CurrentChance=C
	}
}

float UPRBPseudoRandomSubsystem::GetCurrentProb(const UObject* Object) const
{
	if (!Object)
	{
		return 0.0f;
	}

	const FPRBPseudoRandomState* State = FindObjectState(GetUnitID(Object));
	return State ? State->CurrentChance : 0.0f;    // 未初始化过则返回 0
}

int32 UPRBPseudoRandomSubsystem::GetFailCount(const UObject* Object) const
{
	if (!Object)
	{
		return 0;
	}

	const FPRBPseudoRandomState* State = FindObjectState(GetUnitID(Object));
	return State ? State->FailCount : 0;
}

void UPRBPseudoRandomSubsystem::SetSeed(UObject* Object, int32 Seed)
{
	if (!Object)
	{
		UE_LOG(LogPseudoRandom, Warning, TEXT("[PRBPseudoRandomSubsystem] SetSeed: Object is null"));
		return;
	}

	// 设置种子后重置计数器，保证相同种子 + 相同调用序列产生相同结果
	FPRBPseudoRandomState& State = GetOrCreateObjectState(GetUnitID(Object));
	State.RandomStream.Initialize(Seed);
	State.Reset();
}

void UPRBPseudoRandomSubsystem::RemoveObject(UObject* Object)
{
	if (!Object)
	{
		return;
	}

	ObjectPRBPseudoRandomMap.Remove(GetUnitID(Object));
}

// =============================================================
//  GameplayTag Key API 实现
//  与 UObject API 逻辑完全一致，仅 Key 类型不同
// =============================================================

void UPRBPseudoRandomSubsystem::SetExpectedProbabilityForTag(FGameplayTag Tag, float InProbability)
{
	if (!IsValidPseudoRandomTag(Tag))
	{
		UE_LOG(LogPseudoRandom, Warning, TEXT("[PRBPseudoRandomSubsystem] SetExpectedProbabilityForTag: Tag '%s' 不在 Random.Pseudo 层级下"), *Tag.ToString());
		return;
	}

	InProbability = FMath::Clamp(InProbability, 0.0f, 1.0f);
	FPRBPseudoRandomState& State = GetOrCreateTagState(Tag);

	if (FMath::IsNearlyEqual(State.ExpectedProbability, InProbability))
	{
		return;
	}

	State.ExpectedProbability = InProbability;
	State.C = ComputeCValue(InProbability);
	State.Reset();
}

bool UPRBPseudoRandomSubsystem::RollForTag(FGameplayTag Tag)
{
	if (!IsValidPseudoRandomTag(Tag))
	{
		UE_LOG(LogPseudoRandom, Warning, TEXT("[PRBPseudoRandomSubsystem] RollForTag: Tag '%s' 不在 Random.Pseudo 层级下"), *Tag.ToString());
		return false;
	}

	FPRBPseudoRandomState& State = GetOrCreateTagState(Tag);
	return RollInternal(State);
}

void UPRBPseudoRandomSubsystem::ResetForTag(FGameplayTag Tag)
{
	if (!IsValidPseudoRandomTag(Tag))
	{
		UE_LOG(LogPseudoRandom, Warning, TEXT("[PRBPseudoRandomSubsystem] ResetForTag: Tag '%s' 不在 Random.Pseudo 层级下"), *Tag.ToString());
		return;
	}

	if (FPRBPseudoRandomState* State = TagPRBPseudoRandomMap.Find(Tag))
	{
		State->Reset();
	}
}

float UPRBPseudoRandomSubsystem::GetCurrentProbForTag(FGameplayTag Tag) const
{
	if (!IsValidPseudoRandomTag(Tag))
	{
		return 0.0f;
	}

	const FPRBPseudoRandomState* State = FindTagState(Tag);
	return State ? State->CurrentChance : 0.0f;
}

int32 UPRBPseudoRandomSubsystem::GetFailCountForTag(FGameplayTag Tag) const
{
	if (!IsValidPseudoRandomTag(Tag))
	{
		return 0;
	}

	const FPRBPseudoRandomState* State = FindTagState(Tag);
	return State ? State->FailCount : 0;
}

void UPRBPseudoRandomSubsystem::SetSeedForTag(FGameplayTag Tag, int32 Seed)
{
	if (!IsValidPseudoRandomTag(Tag))
	{
		UE_LOG(LogPseudoRandom, Warning, TEXT("[PRBPseudoRandomSubsystem] SetSeedForTag: Tag '%s' 不在 Random.Pseudo 层级下"), *Tag.ToString());
		return;
	}

	FPRBPseudoRandomState& State = GetOrCreateTagState(Tag);
	State.RandomStream.Initialize(Seed);
	State.Reset();
}

void UPRBPseudoRandomSubsystem::RemoveTag(FGameplayTag Tag)
{
	if (!IsValidPseudoRandomTag(Tag))
	{
		UE_LOG(LogPseudoRandom, Warning, TEXT("[PRBPseudoRandomSubsystem] RemoveTag: Tag '%s' 不在 Random.Pseudo 层级下"), *Tag.ToString());
		return;
	}

	TagPRBPseudoRandomMap.Remove(Tag);
}

// =============================================================
//  保底概率 API 实现
//  蓝图可直接调用，消除 PRD 极端连空场景
// =============================================================

void UPRBPseudoRandomSubsystem::SetGuaranteeProbability(UObject* Object, float InGuaranteeProb)
{
	if (!Object)
	{
		UE_LOG(LogPseudoRandom, Warning, TEXT("[PRBPseudoRandomSubsystem] SetGuaranteeProbability: Object is null"));
		return;
	}

	InGuaranteeProb = FMath::Clamp(InGuaranteeProb, 0.0f, 1.0f);

	const int32 UnitID = GetUnitID(Object);
	FPRBPseudoRandomState& State = GetOrCreateObjectState(UnitID);
	State.GuaranteeProbability = InGuaranteeProb;
}

float UPRBPseudoRandomSubsystem::GetGuaranteeProbability(const UObject* Object) const
{
	if (!Object)
	{
		return 0.0f;
	}

	const FPRBPseudoRandomState* State = FindObjectState(GetUnitID(Object));
	return State ? State->GuaranteeProbability : 0.98f;
}

void UPRBPseudoRandomSubsystem::SetGuaranteeProbabilityForTag(FGameplayTag Tag, float InGuaranteeProb)
{
	if (!IsValidPseudoRandomTag(Tag))
	{
		UE_LOG(LogPseudoRandom, Warning, TEXT("[PRBPseudoRandomSubsystem] SetGuaranteeProbabilityForTag: Tag '%s' 不在 Random.Pseudo 层级下"), *Tag.ToString());
		return;
	}

	InGuaranteeProb = FMath::Clamp(InGuaranteeProb, 0.0f, 1.0f);

	FPRBPseudoRandomState& State = GetOrCreateTagState(Tag);
	State.GuaranteeProbability = InGuaranteeProb;
}

float UPRBPseudoRandomSubsystem::GetGuaranteeProbabilityForTag(FGameplayTag Tag) const
{
	if (!IsValidPseudoRandomTag(Tag))
	{
		return 0.0f;
	}

	const FPRBPseudoRandomState* State = FindTagState(Tag);
	return State ? State->GuaranteeProbability : 0.98f;
}


