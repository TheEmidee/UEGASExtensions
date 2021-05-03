#include "GASExtAbilitySystemFunctionLibrary.h"

#include "GASExtAbilityTypesBase.h"
#include "Targeting/GASExtTargetType.h"

#include <AbilitySystemBlueprintLibrary.h>
#include <AbilitySystemComponent.h>
#include <AbilitySystemGlobals.h>

FGASExtGameplayEffectContainerSpec UGASExtAbilitySystemFunctionLibrary::MakeEffectContainerSpecFromEffectContainer( const UGameplayAbility * ability, const FGASExtGameplayEffectContainer & effect_container, const FGameplayEventData & event_data )
{
    FGASExtGameplayEffectContainerSpec container_spec;
    if ( auto * avatar_actor = ability->GetAvatarActorFromActorInfo() )
    {
        container_spec.GameplayEventTags = effect_container.GameplayEventTags;
        container_spec.TargetGameplayEffectSpecs.Reserve( effect_container.TargetGameplayEffectClasses.Num() );

        for ( const auto & gameplay_effect_class : effect_container.TargetGameplayEffectClasses )
        {
            if ( ensureAlwaysMsgf( gameplay_effect_class != nullptr, TEXT( "Can not provide a null class in the TargetEffectClasses of the gameplay effect container" ) ) )
            {
                const auto gameplay_effect_spec_handle = ability->MakeOutgoingGameplayEffectSpec( gameplay_effect_class );
                container_spec.TargetGameplayEffectSpecs.Emplace( gameplay_effect_spec_handle );
            }
        }

        if ( effect_container.TargetTypeClass != nullptr )
        {
            auto * cdo = effect_container.TargetTypeClass->GetDefaultObject< UGASExtTargetType >();
            container_spec.TargetData = cdo->GetTargetData( avatar_actor, FHitResult(), event_data );
        }

        container_spec.EventDataPayload = event_data;
    }
    return container_spec;
}

TArray< FActiveGameplayEffectHandle > UGASExtAbilitySystemFunctionLibrary::ApplyGameplayEffectContainerSpec( const FGASExtGameplayEffectContainerSpec & effect_container_spec )
{
    TArray< FActiveGameplayEffectHandle > applied_gameplay_effect_specs;

    for ( const auto spec_handle : effect_container_spec.TargetGameplayEffectSpecs )
    {
        if ( spec_handle.IsValid() )
        {
            for ( auto target_data : effect_container_spec.TargetData.Data )
            {
                if ( target_data.IsValid() )
                {
                    applied_gameplay_effect_specs.Append( target_data->ApplyGameplayEffectSpec( *spec_handle.Data.Get() ) );
                }
                else
                {
                    UE_LOG( LogTemp, Warning, TEXT( "UGASExtAbilitySystemFunctionLibrary::ApplyGameplayEffectContainerSpec invalid target data passed in." ) );
                }
            }
        }
    }

    for ( const auto event_tag : effect_container_spec.GameplayEventTags )
    {
        for ( const auto & data : effect_container_spec.TargetData.Data )
        {
            for ( auto & target_actor : data->GetActors() )
            {
                if ( auto target_component = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent( target_actor.Get() ) )
                {
                    target_component->HandleGameplayEvent( event_tag, &effect_container_spec.EventDataPayload );
                }
            }
        }
    }

    return applied_gameplay_effect_specs;
}

FGameplayEffectSpecHandle UGASExtAbilitySystemFunctionLibrary::MakeGameplayEffectSpecHandle( const TSubclassOf< UGameplayEffect > effect_class, AActor * instigator, AActor * effect_causer, const UGameplayAbility * ability /* = nullptr */ )
{
    auto effect_context = UAbilitySystemGlobals::Get().AllocGameplayEffectContext();
    effect_context->AddInstigator( instigator, effect_causer );
    effect_context->SetAbility( ability );
    const auto effect_spec = new FGameplayEffectSpec( effect_class.GetDefaultObject(), FGameplayEffectContextHandle( effect_context ), 0 );
    return FGameplayEffectSpecHandle( effect_spec );
}

float UGASExtAbilitySystemFunctionLibrary::GetScalableFloatValue( const FScalableFloat & scalable_float )
{
    return scalable_float.GetValue();
}

bool UGASExtAbilitySystemFunctionLibrary::IsGameplayEffectHandleValid( const FActiveGameplayEffectHandle gameplay_effect_handle )
{
    return gameplay_effect_handle.IsValid();
}
