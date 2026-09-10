// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

//=====================================================================
// ===== [GAS_MOD_10] START=====
// 新增文件：时机（Timing）原生 GameplayTag —— 多时间轴支持
//
// 用 GameplayTag 的层级表达"时间轴 + 时机"，父标签天然聚合子标签：
//
//   TimeAxis
//    ├─ TimeAxis.Round                全体一轮（命名空间，不可作为 GE 的 Timing）
//    │   ├─ TimeAxis.Round.Start      回合开始（每回合一次）
//    │   └─ TimeAxis.Round.End        回合结束（每回合一次，默认时机）
//    └─ TimeAxis.Action               任意出手（父标签，自动覆盖未来新增的 Action.*）
//        ├─ TimeAxis.Action.Attack    攻击出手
//        └─ TimeAxis.Action.Defense   防御出手
//
// 命名取向：Round = 全体一轮，Action = 单个单位的一次出手（层级不重叠）。
//
// 语义约定：
//   - 回合类效果只配叶子（TimeAxis.Round.Start 或 TimeAxis.Round.End），各自每回合一次；
//   - 禁止把 TimeAxis.Round 作为 GE 的 Timing（会被 Start + End 各推一次 = 一回合 2 次）；
//   - 配 TimeAxis.Action 的效果会被"任意出手"（含未来新增的 Action.*）触发。
//=====================================================================

namespace AbilityTimingTags
{
	// ========================================================================
	//  Round Tags（TimeAxis.Round.*）
	// ========================================================================

	/** TimeAxis.Round —— 全体一轮（命名空间标签，不作为 GE 的 Timing 使用，仅用于校验/说明） */
	GAMEPLAYABILITIES_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_TIMEAXIS_ROUND);

	/** TimeAxis.Round.Start —— 回合开始 */
	GAMEPLAYABILITIES_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_TIMEAXIS_ROUND_START);

	/** TimeAxis.Round.End —— 回合结束（未显式配置 Timing 的 GE 的默认时机） */
	GAMEPLAYABILITIES_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_TIMEAXIS_ROUND_END);

	// ========================================================================
	//  Action Tags（TimeAxis.Action.*）
	// ========================================================================

	/** TimeAxis.Action —— 任意出手（父标签，可配，自动覆盖所有 Action.*） */
	GAMEPLAYABILITIES_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_TIMEAXIS_ACTION);

	/** TimeAxis.Action.Attack —— 攻击出手 */
	GAMEPLAYABILITIES_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_TIMEAXIS_ACTION_ATTACK);

	/** TimeAxis.Action.Defense —— 防御出手 */
	GAMEPLAYABILITIES_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_TIMEAXIS_ACTION_DEFENSE);

	/**
	 * 解析 GE 配置的 Timing：无效（未配置）时回落到默认时机 TimeAxis.Round.End。
	 * 用于 GE 注册/查询/清理 Timer 时统一取得实际生效的时机。
	 */
	inline FGameplayTag ResolveOrDefault(const FGameplayTag& InTiming)
	{
		return InTiming.IsValid() ? InTiming : TAG_TIMEAXIS_ROUND_END.GetTag();
	}
}

// ===== [GAS_MOD_10] END =====
//=====================================================================
