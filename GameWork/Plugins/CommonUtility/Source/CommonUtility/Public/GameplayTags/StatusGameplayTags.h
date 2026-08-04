// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "NativeGameplayTags.h"

/**
 * Status 相关 GameplayTag（公开）。
 * 外部请统一通过 #include "GameplayTags/CommonGameplayTags.h" 引入。
 */
namespace CommonGameplayTags
{
	// ========================================================================
	//  Status Death Tags（Status.Death.*）
	// ========================================================================

	COMMONUTILITY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_STATUS_DEATH_DYING);
	COMMONUTILITY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_STATUS_DEATH_DEAD);
};
