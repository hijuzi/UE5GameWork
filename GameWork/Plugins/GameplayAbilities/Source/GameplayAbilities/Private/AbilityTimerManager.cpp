// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilityTimerManager.h"
#include "AbilitySystemComponent.h"

//=====================================================================
// ===== [GAS_MOD_09] START=====
// 新增文件：回合 Timer 管理器实现（TurnBased Support）
//=====================================================================

FAbilityTimerContainer& FAbilityTimerManager::GetAbilityTimerContainer(UAbilitySystemComponent* ASC)
{
	return AbilityTimerContainers.FindOrAdd(TWeakObjectPtr<UAbilitySystemComponent>(ASC));
}

void FAbilityTimerManager::TickTurn(UAbilitySystemComponent* ASC, int32 Delta)
{
	if (!ASC)
	{
		return;
	}

	FAbilityTimerContainer& Container = GetAbilityTimerContainer(ASC);
	for (int32 Step = 0; Step < Delta; ++Step)
	{
		// 推进当前回合
		Container.Turn++;

		// 先收集本轮到期需要移除的 Timer，避免边遍历边删除导致迭代器失效
		TArray<FTimerHandle> ToRemove;
		for (FTimerHandle& Handle : Container.TimerHandles)
		{
			FTimerData* Data = FindTimer(Handle);
			if (!Data)
			{
				// Timer 已被外部清除，直接标记清理容器中的残留句柄
				ToRemove.Emplace(Handle);
				continue;
			}

			if (Data->ExpireTime <= static_cast<double>(Container.Turn))
			{
				// 到期：执行回调（Duration 到期 / Period 触发）
				Data->TimerDelegate.Execute();

				if (Data->bLoop)
				{
					// 循环 Timer：顺延到下一次到期
					Data->ExpireTime = static_cast<double>(Container.Turn) + static_cast<double>(Data->Rate);
				}
				else
				{
					// 一次性 Timer：标记移除
					ToRemove.Emplace(Handle);
				}
			}
		}

		// 统一清理已到期的一次性 Timer
		for (FTimerHandle& Handle : ToRemove)
		{
			RemoveAbilityTimer(ASC, Handle);
			ClearTimer(Handle);
		}
	}
}

void FAbilityTimerManager::SetAbilityTimer(UAbilitySystemComponent* ASC, FTimerHandle& OutHandle, const FTimerDelegate& InDelegate, float InRate, bool bInLoop, float InFirstDelay)
{
	if (!ASC)
	{
		return;
	}

	// 先清理旧 Timer（FTimerManager 内部 + 回合容器）
	ClearTimer(OutHandle);
	RemoveAbilityTimer(ASC, OutHandle);

	if (InRate > 0.f)
	{
		// 复用父类 SetTimer 完成 delegate 注册与 handle 生成
		SetTimer(OutHandle, InDelegate, InRate, bInLoop, InFirstDelay);

		if (OutHandle.IsValid())
		{
			if (FTimerData* Data = FindTimer(OutHandle))
			{
				FAbilityTimerContainer& Container = GetAbilityTimerContainer(ASC);
				const float ActualFirstDelay = (InFirstDelay >= 0.f) ? InFirstDelay : InRate;
				// 关键：把父类 SetTimer 写入的"秒"语义改写为"回合"语义
				Data->ExpireTime = static_cast<double>(Container.Turn) + static_cast<double>(ActualFirstDelay);
				Container.TimerHandles.AddUnique(OutHandle);
			}
		}
	}
}

float FAbilityTimerManager::GetAbilityTimerRemaining(UAbilitySystemComponent* ASC, FTimerHandle& InHandle)
{
	if (!ASC)
	{
		return 0.f;
	}

	FAbilityTimerContainer& Container = GetAbilityTimerContainer(ASC);
	if (!Container.TimerHandles.Contains(InHandle))
	{
		return 0.f;
	}

	FTimerData* Data = FindTimer(InHandle);
	if (!Data)
	{
		return 0.f;
	}

	return static_cast<float>(Data->ExpireTime - static_cast<double>(Container.Turn));
}

int32 FAbilityTimerManager::GetAbilityCurrentTurn(UAbilitySystemComponent* ASC) const
{
	if (const FAbilityTimerContainer* Container = AbilityTimerContainers.Find(TWeakObjectPtr<UAbilitySystemComponent>(ASC)))
	{
		return Container->Turn;
	}
	return 0;
}

void FAbilityTimerManager::RemoveAbilityTimer(UAbilitySystemComponent* ASC, FTimerHandle& InHandle)
{
	if (!ASC)
	{
		return;
	}

	if (FAbilityTimerContainer* Container = AbilityTimerContainers.Find(TWeakObjectPtr<UAbilitySystemComponent>(ASC)))
	{
		Container->TimerHandles.Remove(InHandle);
	}
}

// ===== [GAS_MOD_09] END =====
//=====================================================================
