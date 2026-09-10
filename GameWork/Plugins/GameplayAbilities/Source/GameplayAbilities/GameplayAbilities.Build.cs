// Copyright Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class GameplayAbilities : ModuleRules
	{
		public GameplayAbilities(ReadOnlyTargetRules Target) : base(Target)
		{
			NumIncludedBytesPerUnityCPPOverride = 491520; // best unity size found from using UBT ProfileUnitySizes mode

			//=====================================================================
			// ===== [GAS_MOD_13] START=====
			// 修改前(引擎原版): 无此行。
			// 问题: 引擎源码中 AbilitySystemComponent.cpp include 了
			//   "UObject/UObjectMigrationContext.h"（位于 CoreUObject/Internal）。UBT 规定模块的
			//   Internal 目录只对"其它引擎内部模块"暴露；引擎自带的 GameplayAbilities 属于内部模块
			//   （且以预编译二进制分发），而拷贝到项目内以"项目插件"形式编译时该目录不在 include
			//   路径中，导致 fatal error C1083。
			// 修改后(本项目, 新建): 把依赖模块 CoreUObject 的 Internal 目录加入私有 include 路径。
			//=====================================================================
			PrivateIncludePaths.Add(System.IO.Path.Combine(GetModuleDirectory("CoreUObject"), "Internal"));
			// ===== [GAS_MOD_13] END =====
			//=====================================================================

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"CoreUObject",
					"NetCore",
					"Engine",
					"GameplayTags",
					"GameplayTasks",
					"MovieScene",
					"PhysicsCore",
					"DeveloperSettings",
					"DataRegistry"
				}
				);

			// Niagara support for gameplay cue notifies.
			{
				PrivateDependencyModuleNames.Add("Niagara");
			}

			if (Target.bBuildEditor == true)
			{
				PrivateDependencyModuleNames.Add("EditorFramework");
				PrivateDependencyModuleNames.Add("UnrealEd");
				PrivateDependencyModuleNames.Add("Slate");
				PrivateDependencyModuleNames.Add("SequenceRecorder");
			}

			SetupGameplayDebuggerSupport(Target);

			SetupIrisSupport(Target);
		}
	}
}
