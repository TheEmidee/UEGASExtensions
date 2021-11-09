#include "GASExtAbilitySystemFunctionLibrary.h"

#include "GASExtAbilityTypesBase.h"
#include "Targeting/GASExtTargetType.h"

#include <AbilitySystemBlueprintLibrary.h>
#include <AbilitySystemComponent.h>
#include <AbilitySystemGlobals.h>

FGASExtGameplayEffectContainerSpec UGASExtAbilitySystemFunctionLibrary::MakeEffectContainerSpecFromEffectContainer( const UGameplayAbility * ability, const FGASExtGameplayEffectContainer & effect_container, const FGameplayEventData & event_data )
{
    return MakeEffectContainerSpecFromEffectContainerAndHitResult( ability, effect_container, event_data, FHitResult() );
}

FGASExtGameplayEffectContainerSpec UGASExtAbilitySystemFunctionLibrary::MakeEffectContainerSpecFromEffectContainerAndHitResult( const UGameplayAbility * ability, const FGASExtGameplayEffectContainer & effect_container, const FGameplayEventData & event_data, FHitResult hit_result )
{
    FGASExtGameplayEffectContainerSpec container_spec;
    if ( auto * avatar_actor = ability->GetAvatarActorFromActorInfo() )
    {
        container_spec.GameplayEventTags = effect_container.GameplayEventTags;
        container_spec.TargetGameplayEffectSpecHandles.Reserve( effect_container.TargetGameplayEffectClasses.Num() );

        for ( const auto & gameplay_effect_class : effect_container.TargetGameplayEffectClasses )
        {
            if ( ensureAlwaysMsgf( gameplay_effect_class != nullptr, TEXT( "Can not provide a null class in the TargetEffectClasses of the gameplay effect container" ) ) )
            {
                const auto gameplay_effect_spec_handle = ability->MakeOutgoingGameplayEffectSpec( gameplay_effect_class );
                if ( auto * context = static_cast< FGASExtGameplayEffectContext * >( gameplay_effect_spec_handle.Data->GetContext().Get() ) )
                {
                    context->SetFallOffType( effect_container.FallOffType );

                    if ( effect_container.TargetDataExecutionType == EGASExtGetTargetDataExecutionType::OnEffectContextApplication )
                    {
                        context->SetTargetType( effect_container.TargetType );
                    }
                }
                container_spec.TargetGameplayEffectSpecHandles.Emplace( gameplay_effect_spec_handle );
            }
        }

        if ( effect_container.TargetDataExecutionType == EGASExtGetTargetDataExecutionType::OnEffectContextCreation &&
             effect_container.TargetType != nullptr )
        {
            container_spec.TargetData = effect_container.TargetType->GetTargetData( avatar_actor, hit_result, event_data );
        }

        container_spec.EventDataPayload = event_data;
        container_spec.TargetDataExecutionType = effect_container.TargetDataExecutionType;
    }
    return container_spec;
}

TArray< FActiveGameplayEffectHandle > UGASExtAbilitySystemFunctionLibrary::ApplyGameplayEffectContainerSpec( const FGASExtGameplayEffectContainerSpec & effect_container_spec )
{
    TArray< FActiveGameplayEffectHandle > applied_gameplay_effect_specs;

    // Create a copy of the effect container spec so that we can edit the value of the TargetData in it.
    // Removing const does not work, since that would make the function parameter an output instead of an input in blueprints.
    auto effect_container_spec_copy = effect_container_spec;

    for ( const auto spec_handle : effect_container_spec_copy.TargetGameplayEffectSpecHandles )
    {
        if ( spec_handle.IsValid() )
        {
            if ( const auto * context = static_cast< FGASExtGameplayEffectContext * >( spec_handle.Data->GetContext().Get() ) )
            {
                if ( effect_container_spec_copy.TargetDataExecutionType == EGASExtGetTargetDataExecutionType::OnEffectContextApplication &&
                     context->GetTargetType() != nullptr )
                {
                    effect_container_spec_copy.TargetData.Clear();
                    effect_container_spec_copy.TargetData.Append( context->GetTargetType()->GetTargetData( context->GetEffectCauser(), *context->GetHitResult(), effect_container_spec_copy.EventDataPayload ) );
                }
            }

            for ( auto target_data : effect_container_spec_copy.TargetData.Data )
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

    for ( const auto event_tag : effect_container_spec_copy.GameplayEventTags )
    {
        for ( const auto & data : effect_container_spec_copy.TargetData.Data )
        {
            for ( auto & target_actor : data->GetActors() )
            {
                if ( auto target_component = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent( target_actor.Get() ) )
                {
                    target_component->HandleGameplayEvent( event_tag, &effect_container_spec_copy.EventDataPayload );
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

FGameplayAbilitySpecHandle UGASExtAbilitySystemFunctionLibrary::GiveAbilityAndActivateOnce( UAbilitySystemComponent * asc, TSubclassOf< UGameplayAbility > ability, int32 level /*= 1*/, UObject * source_object /*= nullptr*/ )
{
    FGameplayAbilitySpec spec( ability, level, INDEX_NONE, source_object );
    return asc->GiveAbilityAndActivateOnce( spec );
}

float UGASExtAbilitySystemFunctionLibrary::GetScalableFloatValue( const FScalableFloat & scalable_float )
{
    return scalable_float.GetValue();
}

bool UGASExtAbilitySystemFunctionLibrary::IsGameplayEffectHandleValid( const FActiveGameplayEffectHandle gameplay_effect_handle )
{
    return gameplay_effect_handle.IsValid();
}

FGameplayEffectContextHandle UGASExtAbilitySystemFunctionLibrary::GetContextHandleFromGameplayEffectSpec( const FGameplayEffectSpec & gameplay_effect_spec )
{
    return gameplay_effect_spec.GetContext();
}

FGameplayTagContainer UGASExtAbilitySystemFunctionLibrary::GetTargetTagContainerFromGameplayEffectSpec( const FGameplayEffectSpec & gameplay_effect_spec )
{
    return *gameplay_effect_spec.CapturedTargetTags.GetAggregatedTags();
}
