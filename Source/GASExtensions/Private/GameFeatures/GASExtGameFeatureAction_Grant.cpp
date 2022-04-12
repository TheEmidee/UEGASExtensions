#include "GameFeatures/GASExtGameFeatureAction_Grant.h"

#include "DVEDataValidator.h"

#include <Engine/AssetManager.h>
#include <GameFeaturesSubsystemSettings.h>

#define LOCTEXT_NAMESPACE "UGASExtGameFeatureAction_Grant"

void UGASExtGameFeatureAction_Grant::OnGameFeatureActivating()
{
    if ( !ensureAlways( ActiveExtensions.IsEmpty() ) ||
         !ensureAlways( ComponentRequests.IsEmpty() ) )
    {
        Reset();
    }
    Super::OnGameFeatureActivating();
}

void UGASExtGameFeatureAction_Grant::OnGameFeatureDeactivating( FGameFeatureDeactivatingContext & context )
{
    Super::OnGameFeatureDeactivating( context );
    Reset();
}

#if WITH_EDITORONLY_DATA
void UGASExtGameFeatureAction_Grant::AddAdditionalAssetBundleData( FAssetBundleData & asset_bundle_data )
{
    if ( UAssetManager::IsValid() )
    {
        auto AddBundleAsset = [ &asset_bundle_data ]( const FSoftObjectPath & SoftObjectPath ) {
            asset_bundle_data.AddBundleAsset( UGameFeaturesSubsystemSettings::LoadStateClient, SoftObjectPath );
            asset_bundle_data.AddBundleAsset( UGameFeaturesSubsystemSettings::LoadStateServer, SoftObjectPath );
        };

        for ( const auto & entry : AbilitiesList )
        {
            for ( const auto & ability : entry.GrantedAbilities )
            {
                AddBundleAsset( ability.AbilityType.ToSoftObjectPath() );

                /* :TODO: Re-enable with UE5
                if ( !Ability.InputAction.IsNull() )
                {
                    AddBundleAsset( Ability.InputAction.ToSoftObjectPath() );
                }*/
            }

            for ( const auto & attributes : entry.GrantedAttributes )
            {
                AddBundleAsset( attributes.AttributeSetType.ToSoftObjectPath() );
                if ( !attributes.InitializationData.IsNull() )
                {
                    AddBundleAsset( attributes.InitializationData.ToSoftObjectPath() );
                }
            }

            for ( const auto & effect : entry.GrantedEffects )
            {
                AddBundleAsset( effect.ToSoftObjectPath() );
            }
        }
    }
}
#endif

#if WITH_EDITOR
EDataValidationResult UGASExtGameFeatureAction_Grant::IsDataValid( TArray< FText > & validation_errors )
{
    return FDVEDataValidator( validation_errors )
        .CustomValidation< TArray< FGASExtGameFeatureAbilitiesEntry > >( AbilitiesList, []( TArray< FText > & errors, TArray< FGASExtGameFeatureAbilitiesEntry > abilities_list ) {
            int32 entry_index = 0;
            for ( const auto & entry : abilities_list )
            {
                if ( entry.ActorClass == nullptr )
                {
                    errors.Emplace( FText::Format( LOCTEXT( "EntryHasNullActor", "Null ActorClass at index {0} in AbilitiesList" ), FText::AsNumber( entry_index ) ) );
                }

                if ( entry.GrantedAbilities.Num() == 0 && entry.GrantedAttributes.Num() == 0 )
                {
                    errors.Emplace( FText::Format( LOCTEXT( "EntryHasNoAddOns", "Empty GrantedAbilities and GrantedAttributes at index {0} in AbilitiesList" ), FText::AsNumber( entry_index ) ) );
                }

                int32 ability_index = 0;
                for ( const auto & ability : entry.GrantedAbilities )
                {
                    if ( ability.AbilityType.IsNull() )
                    {
                        errors.Emplace( FText::Format( LOCTEXT( "EntryHasNullAbility", "Null AbilityType at index {0} in AbilitiesList[{1}].GrantedAbilities" ), FText::AsNumber( ability_index ), FText::AsNumber( entry_index ) ) );
                    }
                    ++ability_index;
                }

                int32 attributes_index = 0;
                for ( const auto & attributes : entry.GrantedAttributes )
                {
                    if ( attributes.AttributeSetType.IsNull() )
                    {
                        errors.Emplace( FText::Format( LOCTEXT( "EntryHasNullAttributeSet", "Null AttributeSetType at index {0} in AbilitiesList[{1}].GrantedAttributes" ), FText::AsNumber( attributes_index ), FText::AsNumber( entry_index ) ) );
                    }
                    ++attributes_index;
                }

                int32 effect_index = 0;
                for ( const auto & effect : entry.GrantedEffects )
                {
                    if ( effect.IsNull() )
                    {
                        errors.Emplace( FText::Format( LOCTEXT( "EntryHasNullEffect", "Null Effect at index {0} in AbilitiesList[{1}].GrantedEffects" ), FText::AsNumber( effect_index ), FText::AsNumber( entry_index ) ) );
                    }
                    ++attributes_index;
                }

                ++entry_index;
            }
        } )
        .Result();
}
#endif

void UGASExtGameFeatureAction_Grant::AddToWorld( const FWorldContext & world_context )
{
    if ( const UWorld * world = world_context.World() )
    {
        if ( const UGameInstance * game_instance = world_context.OwningGameInstance )
        {
            if ( world->IsGameWorld() )
            {
                if ( auto * component_manager = UGameInstance::GetSubsystem< UGameFrameworkComponentManager >( game_instance ) )
                {
                    int32 entry_index = 0;
                    for ( const auto & entry : AbilitiesList )
                    {
                        if ( entry.ActorClass.IsNull() )
                        {
                            continue;
                        }

                        UGameFrameworkComponentManager::FExtensionHandlerDelegate AddAbilitiesDelegate = UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(
                            this,
                            &UGameFeatureAction_AddAbilities::HandleActorExtension,
                            entry_index );
                        TSharedPtr< FComponentRequestHandle > ExtensionRequestHandle = component_manager->AddExtensionHandler( entry.ActorClass, AddAbilitiesDelegate );

                        ComponentRequests.Add( ExtensionRequestHandle );
                        entry_index++;
                    }
                }
            }
        }
    }
}

void UGASExtGameFeatureAction_Grant::Reset()
{
}

void UGASExtGameFeatureAction_Grant::HandleActorExtension( AActor * actor, FName event_name, int32 entry_index )
{
}

void UGASExtGameFeatureAction_Grant::AddActorAbilities( AActor * actor, const FGASExtGameFeatureAbilitiesEntry & abilities_entry )
{
}

void UGASExtGameFeatureAction_Grant::RemoveActorAbilities( AActor * actor )
{
}

UActorComponent * UGASExtGameFeatureAction_Grant::FindOrAddComponentForActor( UClass * component_type, AActor * actor, const FGASExtGameFeatureAbilitiesEntry & abilities_entry )
{
}

#undef LOCTEXT_NAMESPACE