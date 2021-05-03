namespace UnrealBuildTool.Rules
{
    public class GASExtensions : ModuleRules
    {
        public GASExtensions( ReadOnlyTargetRules Target )
            : base( Target )
        {
            PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
            bEnforceIWYU = true;

            PrivateIncludePaths.Add("GASExtensions/Private");
            
            PublicDependencyModuleNames.AddRange(
                new string[] { 
                    "Core",
                    "CoreUObject",
                    "Engine",
                    "GameplayAbilities",
                    "GameplayTasks",
                    "GameplayTags",
                    "GameBaseFramework"
                }
            );
        }
    }
}
