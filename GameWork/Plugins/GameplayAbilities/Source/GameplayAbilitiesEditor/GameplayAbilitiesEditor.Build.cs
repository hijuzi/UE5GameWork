// Copyright Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class GameplayAbilitiesEditor : ModuleRules
	{
		public GameplayAbilitiesEditor(ReadOnlyTargetRules Target) : base(Target)
		{
			//=====================================================================
			// ===== [GAS_MOD_13] START=====
			// 修改前(引擎原版): 无此行。
			// 问题: SGameplayCueEditor.cpp include 了 "SAddNewGameplayTagWidget.h"，该头位于
			//   GameplayTagsEditor 模块的 Internal 目录；UBT 规定 Internal 只对"其它引擎内部模块"
			//   暴露，插件形式编译时不可见，导致 fatal error C1083。
			// 修改后(本项目, 新建): 显式把 GameplayTagsEditor/Internal 加入私有 include 路径。
			//=====================================================================
			PrivateIncludePaths.Add(System.IO.Path.Combine(GetModuleDirectory("GameplayTagsEditor"), "Internal"));
			// ===== [GAS_MOD_13] END =====
			//=====================================================================

			PublicDependencyModuleNames.Add("GameplayTasks");

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					// ... add private dependencies that you statically link with here ...
					"AssetDefinition",
					"Core",
					"CoreUObject",
					"Engine",
					"EngineAssetDefinitions",
					"AssetTools",
					"ClassViewer",
					"GameplayTags",
					"GameplayTagsEditor",
					"GameplayAbilities",
					"GameplayTasksEditor",
					"InputCore",
					"PropertyEditor",
					"Slate",
					"SlateCore",					
					"BlueprintGraph",
					"Kismet",
					"KismetCompiler",
					"GraphEditor",
					"LevelSequence",
					"MainFrame",
					"EditorFramework",
					"UnrealEd",
					"WorkspaceMenuStructure",
					"ContentBrowser",
					"EditorWidgets",
					"SourceControl",
					"SequencerCore",
					"Sequencer",
					"MovieSceneTools",
					"MovieScene",
					"DataRegistry",
					"DataRegistryEditor",
					"ToolMenus",
					"ApplicationCore",
				}
			);
		}
	}
}
