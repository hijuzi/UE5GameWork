// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "NativeGameplayTags.h"

/**
 * GameplayEvent 相关 GameplayTag（公开）。
 * 外部请统一通过 #include "GameplayTags/CommonGameplayTags.h" 引入。
 */
namespace CommonGameplayTags
{
	// ========================================================================
	//  GameplayEvent Tags（GameplayEvent.*）
	// ========================================================================

	COMMONUTILITY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GAMEPLAYEVENT_DEATH);
	COMMONUTILITY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GAMEPLAYEVENT_COMBATHUD_SHOW);
	COMMONUTILITY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GAMEPLAYEVENT_COMBATHUD_HIDE);
	COMMONUTILITY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GAMEPLAYEVENT_COMBAT_RESTART);
};
