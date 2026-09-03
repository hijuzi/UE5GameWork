// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "SVCombatHUDLayout.generated.h"

/**
 * 战斗专用 HUD 布局。
 *
 * 继承自 UUserWidget，提供「加载/卸载动画（进场/出场）」与
 * 「显示/隐藏动画」两对扩展入口，用于战斗 HUD 的显隐切换。
 *
 * 两类动画的区别：
 *  - PlayShowAnimation / PlayHideAnimation：显示/隐藏动画（显隐切换）
 *  - PlayLoadAnimation / PlayUnloadAnimation：加载/卸载动画（进场/出场）
 */
UCLASS(Abstract, BlueprintType, Blueprintable, Meta = (DisplayName = "Combat HUD Layout", Category = "CatCombatFramework|HUD"))
class CATCOMBATFRAMEWORK_API USVCombatHUDLayout : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 播放战斗 HUD 显示动画（由蓝图覆写实现具体动画逻辑）。显示时调用。 */
	UFUNCTION(BlueprintNativeEvent, Category = "Combat HUD|Animation")
	void PlayShowAnimation();

	/** 播放战斗 HUD 隐藏动画（由蓝图覆写实现具体动画逻辑）。隐藏时调用。 */
	UFUNCTION(BlueprintNativeEvent, Category = "Combat HUD|Animation")
	void PlayHideAnimation();

	/** 播放战斗 HUD 加载动画（进场，由蓝图覆写实现）。加载时调用。 */
	UFUNCTION(BlueprintNativeEvent, Category = "Combat HUD|Animation")
	void PlayLoadAnimation();

	/** 播放战斗 HUD 卸载动画（出场，由蓝图覆写实现）。卸载时调用。 */
	UFUNCTION(BlueprintNativeEvent, Category = "Combat HUD|Animation")
	void PlayUnloadAnimation();
};
