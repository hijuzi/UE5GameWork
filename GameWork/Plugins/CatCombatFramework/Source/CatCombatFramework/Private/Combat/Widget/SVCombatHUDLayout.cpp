// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/Widget/SVCombatHUDLayout.h"

void USVCombatHUDLayout::PlayShowAnimation_Implementation()
{
	// 默认实现：直接显示 Widget。蓝图可覆写以添加显示动画。
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void USVCombatHUDLayout::PlayHideAnimation_Implementation()
{
	// 默认实现：直接隐藏 Widget。蓝图可覆写以添加隐藏动画。
	SetVisibility(ESlateVisibility::Collapsed);
}

void USVCombatHUDLayout::PlayLoadAnimation_Implementation()
{
	// 默认实现：加载/进场动画，蓝图可覆写实现具体动画逻辑。
}

void USVCombatHUDLayout::PlayUnloadAnimation_Implementation()
{
	// 默认实现：卸载/出场动画，蓝图可覆写实现具体动画逻辑。
}
