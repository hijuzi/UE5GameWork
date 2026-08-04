// Copyright Epic Games, Inc. All Rights Reserved.

#include "CommonUtility.h"

// Tag 定义由 CommonGameplayTags.cpp 中的 UE_DEFINE_GAMEPLAY_TAG_COMMENT 宏自动注册，
// 无需在模块启动时手动调用 AddNativeGameplayTag。

#define LOCTEXT_NAMESPACE "FCommonUtilityModule"

void FCommonUtilityModule::StartupModule()
{
}

void FCommonUtilityModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCommonUtilityModule, CommonUtility)