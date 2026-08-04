// Copyright SegameVictory Team. All Rights Reserved.

using UnrealBuildTool;

public class SVFloatingText : ModuleRules
{
	public SVFloatingText(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"UMG",
			"GameplayTags",
			"DeveloperSettings",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"CommonUtility",
		});
	}
}
