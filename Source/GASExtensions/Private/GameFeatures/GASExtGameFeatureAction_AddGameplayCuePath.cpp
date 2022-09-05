#include "GameFeatures/GASExtGameFeatureAction_AddGameplayCuePath.h"

#include "AbilitySystemGlobals.h"
#include "GameFeatureData.h"
#include "GameplayCueSet.h"
#include "GameplayCues/GASExtGameplayCueManager.h"

#define LOCTEXT_NAMESPACE "GameFeatures"

#if WITH_EDITOR
EDataValidationResult UGASExtGameFeatureAction_AddGameplayCuePath::IsDataValid( TArray< FText > & validation_errors )
{
    EDataValidationResult Result = Super::IsDataValid( validation_errors );

    FText ErrorReason = FText::GetEmpty();
    for ( const FDirectoryPath & Directory : DirectoryPathsToAdd )
    {
        if ( Directory.Path.IsEmpty() )
        {
            const FText InvalidCuePathError = FText::Format( LOCTEXT( "InvalidCuePathError", "'{0}' is not a valid path!" ), FText::FromString( Directory.Path ) );
            validation_errors.Emplace( InvalidCuePathError );
            validation_errors.Emplace( ErrorReason );
            Result = CombineDataValidationResults( Result, EDataValidationResult::Invalid );
        }
    }

    return CombineDataValidationResults( Result, EDataValidationResult::Valid );
}
#endif

void UGASExtGameFeatureObserver_AddGameplayCuePath::OnGameFeatureRegistering( const UGameFeatureData * game_feature_data, const FString & plugin_name )
{
    TRACE_CPUPROFILER_EVENT_SCOPE( ULyraGameFeature_AddGameplayCuePaths::OnGameFeatureRegistering );

    const FString PluginRootPath = TEXT( "/" ) + plugin_name;
    for ( const auto * action : game_feature_data->GetActions() )
    {
        if ( const auto * game_feature_action_add_gameplay_cue_path = Cast< UGASExtGameFeatureAction_AddGameplayCuePath >( action ) )
        {
            const auto & dirs_to_add = game_feature_action_add_gameplay_cue_path->GetDirectoryPathsToAdd();

            if ( auto * gameplay_cue_manager = UGASExtGameplayCueManager::Get() )
            {
                const auto * runtime_gameplay_cue_set = gameplay_cue_manager->GetRuntimeCueSet();
                const auto pre_initialize_num_cues = runtime_gameplay_cue_set ? runtime_gameplay_cue_set->GameplayCueData.Num() : 0;

                for ( const auto & [ path ] : dirs_to_add )
                {
                    auto mutable_path = path;
                    UGameFeaturesSubsystem::FixPluginPackagePath( mutable_path, PluginRootPath, false );
                    gameplay_cue_manager->AddGameplayCueNotifyPath( mutable_path, /** bShouldRescanCueAssets = */ false );
                }

                // Rebuild the runtime library with these new paths
                if ( !dirs_to_add.IsEmpty() )
                {
                    gameplay_cue_manager->InitializeRuntimeObjectLibrary();
                }

                if ( const auto post_initialize_num_cues = runtime_gameplay_cue_set
                                                               ? runtime_gameplay_cue_set->GameplayCueData.Num()
                                                               : 0;
                     pre_initialize_num_cues != post_initialize_num_cues )
                {
                    gameplay_cue_manager->RefreshGameplayCuePrimaryAsset();
                }
            }
        }
    }
}

void UGASExtGameFeatureObserver_AddGameplayCuePath::OnGameFeatureUnregistering( const UGameFeatureData * game_feature_data, const FString & plugin_name )
{
    const FString PluginRootPath = TEXT( "/" ) + plugin_name;
    for ( const UGameFeatureAction * action : game_feature_data->GetActions() )
    {
        if ( const auto * game_feature_action_add_gameplay_cue_path = Cast< UGASExtGameFeatureAction_AddGameplayCuePath >( game_feature_data ) )
        {
            const TArray< FDirectoryPath > & dirs_to_add = game_feature_action_add_gameplay_cue_path->GetDirectoryPathsToAdd();

            if ( auto * gcm = UAbilitySystemGlobals::Get().GetGameplayCueManager() )
            {
                int32 num_removed = 0;
                for ( const auto & [ path ] : dirs_to_add )
                {
                    auto mutable_path = path;
                    UGameFeaturesSubsystem::FixPluginPackagePath( mutable_path, PluginRootPath, false );
                    num_removed += gcm->RemoveGameplayCueNotifyPath( mutable_path, /** bShouldRescanCueAssets = */ false );
                }

                ensure( num_removed == dirs_to_add.Num() );

                // Rebuild the runtime library only if there is a need to
                if ( num_removed > 0 )
                {
                    gcm->InitializeRuntimeObjectLibrary();
                }
            }
        }
    }
}

#undef LOCTEXT_NAMESPACE