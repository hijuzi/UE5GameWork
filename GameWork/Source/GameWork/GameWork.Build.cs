// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GameWork : ModuleRules
{
	public GameWork(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"GameWork",
			"GameWork/Variant_Platforming",
			"GameWork/Variant_Platforming/Animation",
			"GameWork/Variant_Combat",
			"GameWork/Variant_Combat/AI",
			"GameWork/Variant_Combat/Animation",
			"GameWork/Variant_Combat/Gameplay",
			"GameWork/Variant_Combat/Interfaces",
			"GameWork/Variant_Combat/UI",
			"GameWork/Variant_SideScrolling",
			"GameWork/Variant_SideScrolling/AI",
			"GameWork/Variant_SideScrolling/Gameplay",
			"GameWork/Variant_SideScrolling/Interfaces",
			"GameWork/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
