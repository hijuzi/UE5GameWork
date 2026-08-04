// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "NativeGameplayTags.h"

/**
 * 项目中所有 GameplayTag 的统一入口（伞头文件）。
 * 内部按类别拆分到独立文件管理（位于 Public/GameplayTags/ 下）：
 *   - UIGameplayTags.h           UI 相关 Tag
 *   - PlatformGameplayTags.h     Platform 相关 Tag
 *   - GameplayCueGameplayTags.h  GameplayCue 相关 Tag
 *   - RandomGameplayTags.h       随机/伪随机相关 Tag
 *   - EventGameplayTags.h        GameplayEvent 相关 Tag
 *   - StatusGameplayTags.h       Status 相关 Tag
 *
 * 使用方式：
 *   #include "GameplayTags/CommonGameplayTags.h"  // 仅需引入此文件
 *   直接使用 CommonGameplayTags::TAG_XXX 即可，无需魔法字符串。
 */
#include "GameplayTags/UIGameplayTags.h"
#include "GameplayTags/PlatformGameplayTags.h"
#include "GameplayTags/GameplayCueGameplayTags.h"
#include "GameplayTags/RandomGameplayTags.h"
#include "GameplayTags/EventGameplayTags.h"
#include "GameplayTags/StatusGameplayTags.h"

// 本文件不再直接声明任何 Tag，所有声明由上述子文件负责。
