// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameplayTags/GameplayCueGameplayTags.h"

namespace CommonGameplayTags
{
	// ========================================================================
	//  FloatingText Tags（GameplayCue.FloatingText.*）
	// ========================================================================

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GAMEPLAYCUE_FLOATING_TEXT,
		"GameplayCue.FloatingText",
		"浮动文字根标签");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GAMEPLAYCUE_FLOATING_TEXT_DAMAGETAKEN,
		"GameplayCue.FloatingText.DamageTaken",
		"伤害浮动文字");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GAMEPLAYCUE_FLOATING_TEXT_DAMAGETAKEN_CRITDAMAGE,
		"GameplayCue.FloatingText.DamageTaken.CritDamage",
		"暴击伤害浮动文字");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GAMEPLAYCUE_FLOATING_TEXT_HEALING,
		"GameplayCue.FloatingText.Healing",
		"治疗浮动文字");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GAMEPLAYCUE_FLOATING_TEXT_FROZEN,
		"GameplayCue.FloatingText.Frozen",
		"冻结浮动文字");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GAMEPLAYCUE_FLOATING_TEXT_DODGE,
		"GameplayCue.FloatingText.Dodge",
		"闪避浮动文字");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GAMEPLAYCUE_FLOATING_TEXT_BLOCK,
		"GameplayCue.FloatingText.Block",
		"格挡浮动文字");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GAMEPLAYCUE_FLOATING_TEXT_APPLYSTACKFAIL,
		"GameplayCue.FloatingText.Block.ApplyStackFail",
		"应用堆栈失败");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GAMEPLAYCUE_VFX_BOUNCEACTIVE,
		"GameplayCue.VFX.Bounce",
		"反弹特效");

};
