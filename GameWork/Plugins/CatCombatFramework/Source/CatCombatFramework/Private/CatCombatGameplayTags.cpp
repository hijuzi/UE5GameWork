// Copyright Epic Games, Inc. All Rights Reserved.

#include "CatCombatGameplayTags.h"

namespace CatCombatGameplayTags
{
	// 角色回合状态（State.Turn.*）
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_STATE_TURN_IDLE, "State.Turn.Idle", "回合状态-空闲");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_STATE_TURN_SELECTING, "State.Turn.Selecting", "回合状态-选择中");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_STATE_TURN_ACTING, "State.Turn.Acting", "回合状态-行动中");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_STATE_TURN_DEFENDING, "State.Turn.Defending", "回合状态-防守中");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_STATE_TURN_DEFERRED, "State.Turn.Deferred", "回合状态-挂起");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_STATE_TURN_ACTED, "State.Turn.Acted", "回合状态-已行动");

	// 战斗 Ability（Ability.Combat.*）
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_ABILITY_COMBAT_JOIN, "Ability.Combat.Join", "进入战斗 Ability 标签 — 标识角色加入战斗的 Ability");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_ABILITY_COMBAT_LEAVE, "Ability.Combat.Leave", "离开战斗 Ability 标签 — 标识角色脱离战斗的 Ability");
}
