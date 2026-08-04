// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameplayTags/UIGameplayTags.h"

namespace CommonGameplayTags
{
	// ========================================================================
	//  UI Layer Tags
	// ========================================================================

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GAMEUI_LAYER_MASK,
		"GameUI.Layer.Mask",
		"全局遮罩层（队列方式）— 最高层：新手引导等");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GAMEUI_LAYER_WINDOW,
		"GameUI.Layer.Window",
		"顶层弹窗层（栈方式 + 底部遮罩）— 确认框、操作提示、异常提示");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GAMEUI_LAYER_FULLSCREENMENU,
		"GameUI.Layer.FullScreenMenu",
		"全屏菜单层（栈方式）— 登录、设置、存/读档、暂停、章节目录");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GAMEUI_LAYER_GAMEMENU,
		"GameUI.Layer.GameMenu",
		"游戏菜单层（栈方式）— 背包、换装、剧情交互");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GAMEUI_LAYER_HUD,
		"GameUI.Layer.HUD",
		"HUD 层（栈方式）— 准星、血条、技能栏等常驻 UI");

	// ========================================================================
	//  UI Action Tags
	// ========================================================================

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_UI_ACTION_ESCAPE,
		"UI.Action.Escape",
		"Escape / 暂停键动作");
};
