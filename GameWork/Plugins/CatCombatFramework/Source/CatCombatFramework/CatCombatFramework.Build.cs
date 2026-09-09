// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CatCombatFramework : ModuleRules
{
	public CatCombatFramework(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
			"UMG",
			"Slate",
			"SlateCore",
			"DeveloperSettings",
			"AIModule",
			"CommonInput"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
		});
	}
}
