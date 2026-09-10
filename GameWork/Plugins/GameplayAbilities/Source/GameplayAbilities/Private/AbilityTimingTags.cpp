// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilityTimingTags.h"

//=====================================================================
// ===== [GAS_MOD_10] START=====
// 新增文件：时机（Timing）原生 GameplayTag 注册
// 原生 Tag 在模块静态构造时注册，模块卸载时自动注销；无需写入 DefaultGameplayTags.ini。
//=====================================================================

namespace AbilityTimingTags
{
	// ========================================================================
	//  Round Tags（TimeAxis.Round.*）
	// ========================================================================

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_TIMEAXIS_ROUND,
		"TimeAxis.Round",
		"全体一轮（命名空间，不作为 GE 的 Timing 使用）");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_TIMEAXIS_ROUND_START,
		"TimeAxis.Round.Start",
		"回合开始");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_TIMEAXIS_ROUND_END,
		"TimeAxis.Round.End",
		"回合结束（默认时机）");

	// ========================================================================
	//  Action Tags（TimeAxis.Action.*）
	// ========================================================================

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_TIMEAXIS_ACTION,
		"TimeAxis.Action",
		"任意出手（父标签，含未来新增 Action.*）");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_TIMEAXIS_ACTION_ATTACK,
		"TimeAxis.Action.Attack",
		"攻击出手");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_TIMEAXIS_ACTION_DEFENSE,
		"TimeAxis.Action.Defense",
		"防御出手");
}

// ===== [GAS_MOD_10] END =====
//=====================================================================
