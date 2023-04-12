namespace UnrealBuildTool.Rules
{
    public class GASExtensions : ModuleRules
    {
        public GASExtensions( ReadOnlyTargetRules Target )
            : base( Target )
        {
            PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
            bEnforceIWYU = true;
            PrivatePCHHeaderFile = "Private/GASExtensionsPCH.h";

            PrivateIncludePaths.Add("GASExtensions/Private");
            
            PublicDependencyModuleNames.AddRange(
                new string[] { 
                    "Core",
                    "CoreUObject",
                    "Engine",
                    "NetCore",
                    "GameplayAbilities",
                    "GameplayTasks",
                    "GameplayTags",
                    "CoreExtensions",
                    "AIModule",
                    "DataValidationExtensions",
                    "ModularGameplay",
                    "GameFeatures"
                }
            );
        }
    }
}
