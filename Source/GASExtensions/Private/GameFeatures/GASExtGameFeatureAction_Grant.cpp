#include "GameFeatures/GASExtGameFeatureAction_Grant.h"

#include "DVEDataValidator.h"

#include <Engine/AssetManager.h>
#include <GameFeaturesSubsystemSettings.h>

#define LOCTEXT_NAMESPACE "UGASExtGameFeatureAction_Grant"

void UGASExtGameFeatureAction_Grant::OnGameFeatureActivating()
{
    if ( !ensureAlways( ActiveExtensions.Num() == 0 ) ||
         !ensureAlways( ComponentRequests.Num() == 0 ) )
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

                        const UGameFrameworkComponentManager::FExtensionHandlerDelegate add_abilities_delegate = UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(
                            this,
                            &ThisClass::HandleActorExtension,
                            entry_index );
                        auto extension_request_handle = component_manager->AddExtensionHandler( entry.ActorClass, add_abilities_delegate );

                        ComponentRequests.Emplace( extension_request_handle );
                        entry_index++;
                    }
                }
            }
        }
    }
}

void UGASExtGameFeatureAction_Grant::Reset()
{
    while ( ActiveExtensions.Num() > 0 )
    {
        const auto extension_iterator = ActiveExtensions.CreateIterator();
        RemoveActorAbilities( extension_iterator->Key );
    }

    ComponentRequests.Empty();
}

void UGASExtGameFeatureAction_Grant::HandleActorExtension( AActor * actor, FName event_name, int32 entry_index )
{
    if ( !AbilitiesList.IsValidIndex( entry_index ) )
    {
        return;
    }

    const auto & entry = AbilitiesList[ entry_index ];

    if ( event_name == UGameFrameworkComponentManager::NAME_ExtensionRemoved || event_name == UGameFrameworkComponentManager::NAME_ReceiverRemoved )
    {
        RemoveActorAbilities( actor );
    }
    else if ( event_name == UGameFrameworkComponentManager::NAME_ExtensionAdded || event_name == UGameFrameworkComponentManager::NAME_GameActorReady )
    {
        AddActorAbilities( actor, entry );
    }
}

void UGASExtGameFeatureAction_Grant::AddActorAbilities( AActor * actor, const FGASExtGameFeatureAbilitiesEntry & abilities_entry )
{
    if ( auto * ability_system_component = FindOrAddComponentForActor< UAbilitySystemComponent >( actor, abilities_entry ) )
    {
        FActorExtensions AddedExtensions;
        AddedExtensions.Abilities.Reserve( abilities_entry.GrantedAbilities.Num() );
        AddedExtensions.Attributes.Reserve( abilities_entry.GrantedAttributes.Num() );

        for ( const auto & ability : abilities_entry.GrantedAbilities )
        {
            if ( !ability.AbilityType.IsNull() )
            {
                FGameplayAbilitySpec new_ability_spec( ability.AbilityType.LoadSynchronous() );
                auto ability_handle = ability_system_component->GiveAbility( new_ability_spec );

                /*if ( !ability.InputAction.IsNull() )
                {
                    UAbilityInputBindingComponent * InputComponent = FindOrAddComponentForActor< UAbilityInputBindingComponent >( actor, abilities_entry );
                    if ( InputComponent )
                    {
                        InputComponent->SetInputBinding( ability.InputAction.LoadSynchronous(), AbilityHandle );
                    }
                    else
                    {
                        UE_LOG( LogAncientGame, Error, TEXT( "Failed to find/add an ability input binding component to '%s' -- are you sure it's a pawn class?" ), *actor->GetPathName() );
                    }
                }*/

                AddedExtensions.Abilities.Add( ability_handle );
            }
        }

        for ( const auto & attributes : abilities_entry.GrantedAttributes )
        {
            if ( !attributes.AttributeSetType.IsNull() )
            {
                TSubclassOf< UAttributeSet > attribute_set_class = attributes.AttributeSetType.LoadSynchronous();
                if ( attribute_set_class != nullptr )
                {
                    UAttributeSet * new_set = NewObject< UAttributeSet >( ability_system_component, attribute_set_class );
                    if ( !attributes.InitializationData.IsNull() )
                    {
                        if ( UDataTable * init_data = attributes.InitializationData.LoadSynchronous() )
                        {
                            new_set->InitFromMetaDataTable( init_data );
                        }
                    }

                    AddedExtensions.Attributes.Add( new_set );
                    ability_system_component->AddAttributeSetSubobject( new_set );
                }
            }
        }

        for ( const auto & effect : abilities_entry.GrantedEffects )
        {
            if ( !effect.IsNull() )
            {
                TSubclassOf< UGameplayEffect > gameplay_effect_class = effect.LoadSynchronous();

                if ( gameplay_effect_class != nullptr )
                {
                    auto effect_context = ability_system_component->MakeEffectContext();
                    const auto spec_handle = ability_system_component->MakeOutgoingSpec( gameplay_effect_class, 1, effect_context );
                    const auto active_effect_handle = ability_system_component->BP_ApplyGameplayEffectSpecToSelf( spec_handle );
                    AddedExtensions.Effects.Add( active_effect_handle );
                }
            }
        }

        ability_system_component->AddLooseGameplayTags( abilities_entry.LooseGameplayTags );
        AddedExtensions.Tags.AppendTags( abilities_entry.LooseGameplayTags );

        ActiveExtensions.Add( actor, AddedExtensions );
    }
}

void UGASExtGameFeatureAction_Grant::RemoveActorAbilities( AActor * actor )
{
    if ( auto * actor_extensions = ActiveExtensions.Find( actor ) )
    {
        if ( auto * ability_system_component = actor->FindComponentByClass< UAbilitySystemComponent >() )
        {
            for ( auto * attribute_set : actor_extensions->Attributes )
            {
                ability_system_component->GetSpawnedAttributes_Mutable().Remove( attribute_set );
            }

            /* :TODO: Un comment with UE5
            UAbilityInputBindingComponent * InputComponent = actor->FindComponentByClass< UAbilityInputBindingComponent >();
            */
            for ( const auto ability_handle : actor_extensions->Abilities )
            {
                /*if ( InputComponent )
                {
                    InputComponent->ClearInputBinding( AbilityHandle );
                }*/
                ability_system_component->SetRemoveAbilityOnEnd( ability_handle );
            }

            for ( const auto effect_handle : actor_extensions->Effects )
            {
                ability_system_component->RemoveActiveGameplayEffect( effect_handle );
            }

            ability_system_component->RemoveLooseGameplayTags( actor_extensions->Tags );
        }

        ActiveExtensions.Remove( actor );
    }
}

UActorComponent * UGASExtGameFeatureAction_Grant::FindOrAddComponentForActor( UClass * component_type, AActor * actor, const FGASExtGameFeatureAbilitiesEntry & abilities_entry )
{
    auto * component = actor->FindComponentByClass( component_type );

    bool make_component_request = component == nullptr;
    if ( component != nullptr )
    {
        // Check to see if this component was created from a different `UGameFrameworkComponentManager` request.
        // `Native` is what `CreationMethod` defaults to for dynamically added components.
        if ( component->CreationMethod == EComponentCreationMethod::Native )
        {
            // Attempt to tell the difference between a true native component and one created by the GameFrameworkComponent system.
            // If it is from the UGameFrameworkComponentManager, then we need to make another request (requests are ref counted).
            if ( const auto * component_archetype = component->GetArchetype() )
            {
                make_component_request = component_archetype->HasAnyFlags( RF_ClassDefaultObject );
            }
        }
    }

    if ( make_component_request )
    {
        const auto * world = actor->GetWorld();
        const auto * game_instance = world->GetGameInstance();

        if ( auto * component_manager = UGameInstance::GetSubsystem< UGameFrameworkComponentManager >( game_instance ) )
        {
            const TSharedPtr< FComponentRequestHandle > request_handle = component_manager->AddComponentRequest( abilities_entry.ActorClass, component_type );
            ComponentRequests.Add( request_handle );
        }

        if ( component == nullptr )
        {
            component = actor->FindComponentByClass( component_type );
            ensureAlways( component );
        }
    }

    return component;
}

#undef LOCTEXT_NAMESPACE