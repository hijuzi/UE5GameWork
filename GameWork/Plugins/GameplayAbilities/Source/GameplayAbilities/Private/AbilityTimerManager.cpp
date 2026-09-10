// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilityTimerManager.h"
#include "AbilitySystemComponent.h"
#include "AbilityTimingTags.h"
#include "AbilitySystemLog.h"

//=====================================================================
// ===== [GAS_MOD_09] START=====
// 新增文件：时间轴 Timer 管理器实现（TurnBased Support + 多时间轴）
// 多时间轴改造已并入本编号 GAS_MOD_09。
//=====================================================================

FAbilityTimerContainer& FAbilityTimerManager::GetAbilityTimerContainer(UAbilitySystemComponent* ASC)
{
	return AbilityTimerContainers.FindOrAdd(TWeakObjectPtr<UAbilitySystemComponent>(ASC));
}

FAbilityTimerAxis& FAbilityTimerManager::GetAbilityTimerAxis(UAbilitySystemComponent* ASC, FGameplayTag InTiming)
{
	FAbilityTimerContainer& Container = GetAbilityTimerContainer(ASC);
	return Container.Axes.FindOrAdd(InTiming);
}

void FAbilityTimerManager::TickTimeline(UAbilitySystemComponent* ASC, FGameplayTag InTiming, int32 Delta)
{
	if (!ASC || !InTiming.IsValid())
	{
		return;
	}

	FAbilityTimerContainer& Container = GetAbilityTimerContainer(ASC);

	for (int32 Step = 0; Step < Delta; ++Step)
	{
		// 先快照"被本次推进命中"的轴 Tag（InTiming 是轴 Tag 的子标签或相等）：
		//   InTiming = TimeAxis.Round.End     → 命中 Round.End 轴
		//   InTiming = TimeAxis.Action.Attack → 命中 Action.Attack 轴、Action 轴（父级聚合）
		// 快照是为了避免回调执行过程中容器被修改（例如回调里移除了 GE）导致迭代器失效。
		TArray<FGameplayTag> MatchedAxes;
		for (const TPair<FGameplayTag, FAbilityTimerAxis>& Pair : Container.Axes)
		{
			if (InTiming.MatchesTag(Pair.Key))
			{
				MatchedAxes.Emplace(Pair.Key);
			}
		}

		for (const FGameplayTag& AxisTag : MatchedAxes)
		{
			FAbilityTimerAxis* Axis = Container.Axes.Find(AxisTag);
			if (!Axis)
			{
				// 轴在本步进内已被移除
				continue;
			}

			// 推进该轴刻度
			const int32 CurrentCounter = ++Axis->Counter;

			// 快照该轴的句柄数组：回调中可能移除 GE 从而修改此数组
			const TArray<FTimerHandle> Handles = Axis->TimerHandles;

			for (FTimerHandle Handle : Handles)
			{
				FTimerData* Data = FindTimer(Handle);
				if (!Data)
				{
					// Timer 已被外部清除，清理容器中的残留句柄
					RemoveAbilityTimer(ASC, AxisTag, Handle);
					continue;
				}

				if (Data->ExpireTime <= static_cast<double>(CurrentCounter))
				{
					// 先取快照再执行：回调（如 Duration 到期移除 GE）可能让 Data 失效
					const bool bLoop = Data->bLoop;
					const float Rate = Data->Rate;

					// 到期：执行回调（Duration 到期 / Period 触发）。
					// 注意：FTimerUnifiedDelegate::Execute() 未带 ENGINE_API（引擎内部同 DLL 可直接调用），
					// 插件跨 DLL 调用会报 LNK2019；这里改为取内部 VariantDelegate 中的 FTimerDelegate 直接执行
					//（TVariant::TryGet 与 TDelegate::Execute 均为 inline，无链接依赖）。
					if (FTimerDelegate* NativeDelegate = Data->TimerDelegate.VariantDelegate.TryGet<FTimerDelegate>())
					{
						NativeDelegate->Execute();
					}

					if (bLoop)
					{
						// 循环 Timer：顺延到下一次到期（回调若已清除该 Timer，则不再顺延）
						if (FTimerData* DataAfter = FindTimer(Handle))
						{
							DataAfter->ExpireTime = static_cast<double>(CurrentCounter) + static_cast<double>(Rate);
						}
					}
					else
					{
						// 一次性 Timer：标记移除
						RemoveAbilityTimer(ASC, AxisTag, Handle);
						ClearTimer(Handle);
					}
				}
			}
		}
	}
}

void FAbilityTimerManager::SetAbilityTimer(UAbilitySystemComponent* ASC, FGameplayTag InTiming, FTimerHandle& OutHandle, const FTimerDelegate& InDelegate, float InRate, bool bInLoop, float InFirstDelay)
{
	if (!ASC)
	{
		return;
	}

	// 时机必须有效才能建轴
	if (!InTiming.IsValid())
	{
		UE_LOG(LogGameplayEffects, Warning, TEXT("[GAS_MOD_09] SetAbilityTimer 收到无效 Timing，已忽略注册。ASC=%s"), *GetNameSafe(ASC));
		return;
	}

	// 语义兜底：TimeAxis.Round 是命名空间标签，不可作为 GE 的 Timing
	// （被 Round.Start + Round.End 各推一次 = 一回合结算 2 次）
	if (InTiming == AbilityTimingTags::TAG_TIMEAXIS_ROUND.GetTag())
	{
		UE_LOG(LogGameplayEffects, Warning, TEXT("[GAS_MOD_09] GE 的 Timing 不应配置命名空间标签 TimeAxis.Round，请改配 TimeAxis.Round.Start 或 TimeAxis.Round.End。ASC=%s"), *GetNameSafe(ASC));
	}

	// 先清理旧 Timer（FTimerManager 内部 + 所有轴上的旧句柄）
	ClearTimer(OutHandle);
	RemoveAbilityTimerByHandle(ASC, OutHandle);

	if (InRate > 0.f)
	{
		// 复用父类 SetTimer 完成 delegate 注册与 handle 生成
		SetTimer(OutHandle, InDelegate, InRate, bInLoop, InFirstDelay);

		if (OutHandle.IsValid())
		{
			if (FTimerData* Data = FindTimer(OutHandle))
			{
				FAbilityTimerAxis& Axis = GetAbilityTimerAxis(ASC, InTiming);
				const float ActualFirstDelay = (InFirstDelay >= 0.f) ? InFirstDelay : InRate;
				// 关键：把父类 SetTimer 写入的"秒"语义改写为"该时机刻度"语义
				Data->ExpireTime = static_cast<double>(Axis.Counter) + static_cast<double>(ActualFirstDelay);
				Axis.TimerHandles.AddUnique(OutHandle);
			}
		}
	}
}

float FAbilityTimerManager::GetAbilityTimerRemaining(UAbilitySystemComponent* ASC, FGameplayTag InTiming, FTimerHandle& InHandle)
{
	if (!ASC)
	{
		return 0.f;
	}

	const FAbilityTimerContainer* Container = AbilityTimerContainers.Find(TWeakObjectPtr<UAbilitySystemComponent>(ASC));
	if (!Container)
	{
		return 0.f;
	}

	const FAbilityTimerAxis* Axis = Container->Axes.Find(InTiming);
	if (!Axis || !Axis->TimerHandles.Contains(InHandle))
	{
		return 0.f;
	}

	FTimerData* Data = FindTimer(InHandle);
	if (!Data)
	{
		return 0.f;
	}

	return static_cast<float>(Data->ExpireTime - static_cast<double>(Axis->Counter));
}

int32 FAbilityTimerManager::GetTimelineCounter(UAbilitySystemComponent* ASC, FGameplayTag InTiming) const
{
	if (const FAbilityTimerContainer* Container = AbilityTimerContainers.Find(TWeakObjectPtr<UAbilitySystemComponent>(ASC)))
	{
		if (const FAbilityTimerAxis* Axis = Container->Axes.Find(InTiming))
		{
			return Axis->Counter;
		}
	}
	return 0;
}

void FAbilityTimerManager::RemoveAbilityTimer(UAbilitySystemComponent* ASC, FGameplayTag InTiming, FTimerHandle& InHandle)
{
	if (!ASC)
	{
		return;
	}

	FAbilityTimerContainer* Container = AbilityTimerContainers.Find(TWeakObjectPtr<UAbilitySystemComponent>(ASC));
	if (!Container)
	{
		return;
	}

	// 优先在指定时机上移除；若该轴内不存在该句柄（例如 GE 中途更换了 Timing），退化为全局按句柄查找
	if (FAbilityTimerAxis* Axis = Container->Axes.Find(InTiming))
	{
		if (Axis->TimerHandles.Remove(InHandle) > 0)
		{
			return;
		}
	}

	RemoveAbilityTimerByHandle(ASC, InHandle);
}

void FAbilityTimerManager::RemoveAbilityTimerByHandle(UAbilitySystemComponent* ASC, FTimerHandle& InHandle)
{
	if (!ASC || !InHandle.IsValid())
	{
		return;
	}

	FAbilityTimerContainer* Container = AbilityTimerContainers.Find(TWeakObjectPtr<UAbilitySystemComponent>(ASC));
	if (!Container)
	{
		return;
	}

	for (TPair<FGameplayTag, FAbilityTimerAxis>& Pair : Container->Axes)
	{
		Pair.Value.TimerHandles.Remove(InHandle);
	}
}

// ===== [GAS_MOD_09] END =====
//=====================================================================
