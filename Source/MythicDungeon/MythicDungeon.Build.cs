// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MythicDungeon : ModuleRules
{
	public MythicDungeon(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(new string[] {"MythicDungeon"});

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "NavigationSystem", "AIModule", "Niagara", "EnhancedInput", "GameplayAbilities", "GameplayTags", "GameplayTasks", "NetCore", "StarLink" });
    }
}
