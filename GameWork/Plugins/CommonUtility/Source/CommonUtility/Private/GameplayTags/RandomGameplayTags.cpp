// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameplayTags/RandomGameplayTags.h"

namespace CommonGameplayTags
{
	// ========================================================================
	//  伪随机根 Tag（Random.Pseudo）
	// ========================================================================

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_RANDOM_PSEUDO,
		"Random.Pseudo",
		"伪随机根标签");

	// ========================================================================
	//  伪随机测试 Tag（Random.Pseudo.Test）
	// ========================================================================

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_RANDOM_PSEUDO_TEST,
		"Random.Pseudo.Test",
		"伪随机批量测试用标签，所有测试实例复用此 Tag（通过 Reset/Remove 隔离状态）");

	// ========================================================================
	//  伪随机 Ability Tags（Random.Pseudo.Ability.*）
	// ========================================================================

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_RANDOM_PSEUDO_ABILITY_DODGE,
		"Random.Pseudo.Ability.Dodge",
		"闪避概率判定（伪随机）");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_RANDOM_PSEUDO_ABILITY_CRITICALHIT,
		"Random.Pseudo.Ability.CriticalHit",
		"暴击概率判定（伪随机）");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_RANDOM_PSEUDO_ABILITY_PASSIVETRIGGER,
		"Random.Pseudo.Ability.PassiveTrigger",
		"被动触发概率判定（伪随机）");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_RANDOM_PSEUDO_ABILITY_PROC,
		"Random.Pseudo.Ability.Proc",
		"技能触发概率判定（伪随机）");
};
