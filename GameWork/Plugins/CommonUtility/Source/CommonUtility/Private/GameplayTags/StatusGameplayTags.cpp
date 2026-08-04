// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameplayTags/StatusGameplayTags.h"

namespace CommonGameplayTags
{
	// ========================================================================
	//  Status Death Tags（Status.Death.*）
	// ========================================================================

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_STATUS_DEATH_DYING,
		"Status.Death.Dying",
		"正在死亡中 — 死亡流程已开始，等待动画/布娃娃结束");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_STATUS_DEATH_DEAD,
		"Status.Death.Dead",
		"死亡已完成 — 死亡流程结束，角色处于最终死亡状态");
};
