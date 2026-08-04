// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "NativeGameplayTags.h"

/**
 * 随机 / 伪随机相关 GameplayTag（公开）。
 * 外部请统一通过 #include "GameplayTags/CommonGameplayTags.h" 引入。
 */
namespace CommonGameplayTags
{
	// ========================================================================
	//  伪随机根 Tag（Random.Pseudo）
	// ========================================================================

	COMMONUTILITY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_RANDOM_PSEUDO);

	// ========================================================================
	//  伪随机测试 Tag（Random.Pseudo.Test）
	// ========================================================================

	COMMONUTILITY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_RANDOM_PSEUDO_TEST);

	// ========================================================================
	//  伪随机 Ability Tags（Random.Pseudo.Ability.*）
	// ========================================================================

	COMMONUTILITY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_RANDOM_PSEUDO_ABILITY_DODGE);
	COMMONUTILITY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_RANDOM_PSEUDO_ABILITY_CRITICALHIT);
	COMMONUTILITY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_RANDOM_PSEUDO_ABILITY_PASSIVETRIGGER);
	COMMONUTILITY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_RANDOM_PSEUDO_ABILITY_PROC);
};
