// Copyright SegameVictory. All Rights Reserved.
// 作者: LiuYang
// 创建日期: 2026-07-27
// 功能: 不受 GlobalTimeDilation 影响的真实世界时间延迟异步节点

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "SVAsyncAction_DelayUnscaled.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDelayUnscaledCompleted);

/**
 * 真实世界时间延迟异步节点
 *
 * 与标准 Delay 节点不同：
 *  - 使用 FPlatformTime::Seconds() 计算经过时间
 *  - 完全不受 GlobalTimeDilation 影响
 *  - 仅输出 Completed 执行引脚（隐藏默认 Then 引脚）
 */
UCLASS(meta = (HideThen = "true"))
class CATCOMBATFRAMEWORK_API USVAsyncAction_DelayUnscaled : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/** 延迟结束后执行的事件 */
	UPROPERTY(BlueprintAssignable)
	FOnDelayUnscaledCompleted Completed;

	/**
	 * 创建一个使用真实世界时间的延迟节点
	 * @param WorldContextObject 世界上下文对象（自动填充为 self）
	 * @param Duration 延迟时长（秒），使用真实世界时间计算，不受 GlobalTimeDilation 影响
	 * @return 异步节点实例
	 */
	UFUNCTION(BlueprintCallable, Category = "Combat", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject", DisplayName = "Delay Unscaled"))
	static USVAsyncAction_DelayUnscaled* DelayUnscaled(UObject* WorldContextObject, float Duration = 1.0f);

	//~ Begin UBlueprintAsyncActionBase interface
	virtual void Activate() override;
	//~ End UBlueprintAsyncActionBase interface

private:
	/** 定时器到期回调，广播 Completed 并标记可销毁 */
	void OnTimerComplete();

	/** 世界上下文对象 */
	UPROPERTY()
	TObjectPtr<UObject> WorldContextObject;

	/** 延迟时长（秒） */
	float Duration = 0.0f;
};
