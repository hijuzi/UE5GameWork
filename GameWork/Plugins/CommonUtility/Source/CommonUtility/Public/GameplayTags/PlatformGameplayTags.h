// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "NativeGameplayTags.h"

/**
 * Platform 相关 GameplayTag（公开）。
 * 外部请统一通过 #include "GameplayTags/CommonGameplayTags.h" 引入。
 */
namespace CommonGameplayTags
{
	// ========================================================================
	//  Platform Tags
	// ========================================================================

	COMMONUTILITY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PLATFORM_TRAIT_INPUT_PRIMARILYCONTROLLER);
};
