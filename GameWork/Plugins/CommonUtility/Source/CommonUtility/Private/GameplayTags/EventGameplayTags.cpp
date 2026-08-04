// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameplayTags/EventGameplayTags.h"

namespace CommonGameplayTags
{
	// ========================================================================
	//  GameplayEvent Tags（GameplayEvent.*）
	// ========================================================================

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GAMEPLAYEVENT_DEATH,
		"GameplayEvent.Death",
		"死亡事件标签 — 通过 ASC 发送，驱动死亡 GA/动画/布娃娃流程");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GAMEPLAYEVENT_COMBATHUD_SHOW,
		"GameplayEvent.CombatHUD.Show",
		"战斗HUD显示事件 — 通过 GameplayMessageSubsystem 广播");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GAMEPLAYEVENT_COMBATHUD_HIDE,
		"GameplayEvent.CombatHUD.Hide",
		"战斗HUD隐藏事件 — 通过 GameplayMessageSubsystem 广播");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GAMEPLAYEVENT_COMBAT_RESTART,
		"GameplayEvent.Combat.Restart",
		"战斗重新挑战事件 — 重新开始当前战斗");
};
