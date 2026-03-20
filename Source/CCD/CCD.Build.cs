// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CCD : ModuleRules
{
	public CCD(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"AIModule",
			"GameplayTasks",
			"NavigationSystem",
			"EnhancedInput",
			"ActorSequence", 
			"MovieScene", 
			"LevelSequence", 
			"OnlineSubsystem",
			"OnlineSubsystemUtils"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"UMG",        // UUserWidget, UButton, UEditableTextBox 등
			"Slate",      // Slate 위젯 사용 시 필요
			"SlateCore", 
			"Niagara",
			"GeometryCollectionEngine",
			"ChaosSolverEngine"
		});
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
