// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "NativeGameplayTags.h"

/**
 * UI 相关 GameplayTag（公开）。
 * 外部请统一通过 #include "GameplayTags/CommonGameplayTags.h" 引入。
 */
namespace CommonGameplayTags
{
	// ========================================================================
	//  UI Layer Tags
	// ========================================================================

	COMMONUTILITY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GAMEUI_LAYER_MASK);
	COMMONUTILITY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GAMEUI_LAYER_WINDOW);
	COMMONUTILITY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GAMEUI_LAYER_FULLSCREENMENU);
	COMMONUTILITY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GAMEUI_LAYER_GAMEMENU);
	COMMONUTILITY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GAMEUI_LAYER_HUD);

	// ========================================================================
	//  UI Action Tags
	// ========================================================================

	COMMONUTILITY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_UI_ACTION_ESCAPE);
};
