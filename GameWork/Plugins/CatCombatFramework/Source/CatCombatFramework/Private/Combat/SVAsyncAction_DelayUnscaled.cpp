// Copyright SegameVictory. All Rights Reserved.
// 作者: LiuYang
// 创建日期: 2026-07-27
// 功能: 不受 GlobalTimeDilation 影响的真实世界时间延迟异步节点实现

#include "Combat/SVAsyncAction_DelayUnscaled.h"

#include "Containers/Ticker.h"

USVAsyncAction_DelayUnscaled* USVAsyncAction_DelayUnscaled::DelayUnscaled(UObject* WorldContextObject, float Duration)
{
	USVAsyncAction_DelayUnscaled* Node = NewObject<USVAsyncAction_DelayUnscaled>();
	Node->WorldContextObject = WorldContextObject;
	Node->Duration = Duration;

	// 将节点注册到 GameInstance 以维持生命周期，避免被 GC 回收
	Node->RegisterWithGameInstance(WorldContextObject);
	return Node;
}

void USVAsyncAction_DelayUnscaled::Activate()
{
	// Duration <= 0 或 WorldContextObject 无效时，立即触发 Completed 并销毁节点
	if (Duration <= 0.0f || !WorldContextObject)
	{
		Completed.Broadcast();
		SetReadyToDestroy();
		return;
	}

	// 使用 FPlatformTime::Seconds() 计算目标时间点（真实世界时间，不受 GlobalTimeDilation 影响）
	double EndTime = FPlatformTime::Seconds() + Duration;
	TWeakObjectPtr<USVAsyncAction_DelayUnscaled> WeakThis(this);

	// 向核心 Ticker 注册每帧回调，轮询检查是否到达目标时间
	FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateLambda([WeakThis, EndTime](float DeltaTime) -> bool
		{
			if (FPlatformTime::Seconds() >= EndTime)
			{
				// 时间到达，触发 Completed 并移除当前 Ticker
				if (WeakThis.IsValid())
				{
					WeakThis->OnTimerComplete();
				}
				return false; // 返回 false 以自动从 Ticker 中移除
			}
			return true; // 继续等待
		}),
		0.0f // 每帧执行一次
	);
}

void USVAsyncAction_DelayUnscaled::OnTimerComplete()
{
	Completed.Broadcast();
	SetReadyToDestroy();
}
