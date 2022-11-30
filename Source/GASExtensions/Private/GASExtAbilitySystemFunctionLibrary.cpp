#include "GASExtAbilitySystemFunctionLibrary.h"

#include "GASExtAbilityTypesBase.h"
#include "Targeting/GASExtTargetDataFilter.h"
#include "Targeting/GASExtTargetDataGenerator.h"

#include <AbilitySystemBlueprintLibrary.h>
#include <AbilitySystemComponent.h>
#include <AbilitySystemGlobals.h>

FGASExtGameplayEffectContainerSpec UGASExtAbilitySystemFunctionLibrary::MakeEffectContainerSpecFromEffectContainer( const UGameplayAbility * ability, const FGASExtGameplayEffectContainer & effect_container, const FGameplayAbilityTargetDataHandle & target_data, const FGameplayEventData & event_data, int level /* = 1 */ )
{
    FGASExtGameplayEffectContainerSpec container_spec;
    if ( auto * avatar_actor = ability->GetAvatarActorFromActorInfo() )
    {
        container_spec.GameplayEventTags = effect_container.GameplayEventTags;
        container_spec.TargetData.Append( target_data );

        if ( ensureAlwaysMsgf( effect_container.GameplayEffect != nullptr, TEXT( "Can not provide a null class in the TargetEffectClasses of the gameplay effect container" ) ) )
        {
            container_spec.GameplayEffectSpecHandle = ability->MakeOutgoingGameplayEffectSpec( effect_container.GameplayEffect, static_cast< float >( level ) );

            for ( const auto & [ tag, magnitude ] : effect_container.SetByCallerTagsToMagnitudeMap )
            {
                container_spec.GameplayEffectSpecHandle.Data.Get()->SetSetByCallerMagnitude( tag, magnitude.GetValue() );
            }

            if ( auto * gameplay_effect_context = container_spec.GameplayEffectSpecHandle.Data->GetContext().Get() )
            {
                if ( auto * gas_ext_gameplay_effect_context = static_cast< FGASExtGameplayEffectContext * >( gameplay_effect_context ) )
                {
                    gas_ext_gameplay_effect_context->SetFallOffType( effect_container.FallOffType );

                    if ( effect_container.TargetDataGenerationPhase == EGASExtTargetDataGenerationPhase::OnEffectContextApplication )
                    {
                        gas_ext_gameplay_effect_context->SetAdditionalTargetDataGenerator( effect_container.AdditionalTargetDataGenerator );
                    }
                }

                if ( effect_container.TargetDataGenerationPhase == EGASExtTargetDataGenerationPhase::OnEffectContextCreation &&
                     effect_container.AdditionalTargetDataGenerator != nullptr )
                {
                    container_spec.TargetData.Append( effect_container.AdditionalTargetDataGenerator->GetTargetData( gameplay_effect_context, event_data ) );
                }
            }
        }

        container_spec.EventDataPayload = event_data;
        container_spec.TargetDataExecutionType = effect_container.TargetDataGenerationPhase;
    }
    return container_spec;
}

TArray< FActiveGameplayEffectHandle > UGASExtAbilitySystemFunctionLibrary::MakeAndApplyEffectContainerSpecFromEffectContainer( const UGameplayAbility * ability, const FGASExtGameplayEffectContainer & effect_container, const FGameplayAbilityTargetDataHandle & target_data, const FGameplayEventData & event_data, int level )
{
    auto container_spec = MakeEffectContainerSpecFromEffectContainer( ability, effect_container, target_data, event_data, level );
    return ApplyGameplayEffectContainerSpec( container_spec );
}

TArray< FActiveGameplayEffectHandle > UGASExtAbilitySystemFunctionLibrary::ApplyGameplayEffectContainerSpec( FGASExtGameplayEffectContainerSpec & effect_container_spec )
{
    TArray< FActiveGameplayEffectHandle > applied_gameplay_effect_specs;

    FGameplayAbilityTargetDataHandle target_data_handle;

    const auto spec_handle = effect_container_spec.GameplayEffectSpecHandle;

    if ( spec_handle.IsValid() )
    {
        if ( const auto * context = static_cast< FGASExtGameplayEffectContext * >( spec_handle.Data->GetContext().Get() ) )
        {
            if ( effect_container_spec.TargetDataExecutionType == EGASExtTargetDataGenerationPhase::OnEffectContextApplication &&
                 context->GetAdditionalTargetDataGenerator() != nullptr )
            {
                target_data_handle.Append( context->GetAdditionalTargetDataGenerator()->GetTargetData( context, effect_container_spec.EventDataPayload ) );
            }
            else
            {
                target_data_handle = effect_container_spec.TargetData;
            }

            if ( const auto * target_data_filter = context->GetTargetDataFilter() )
            {
                target_data_handle = target_data_filter->FilterTargetData( target_data_handle );
            }
        }

        for ( auto target_data : target_data_handle.Data )
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

    for ( const auto event_tag : effect_container_spec.GameplayEventTags )
    {
        for ( const auto & data : target_data_handle.Data )
        {
            for ( auto & target_actor : data->GetActors() )
            {
                if ( auto * target_component = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent( target_actor.Get() ) )
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

FGameplayAbilitySpecHandle UGASExtAbilitySystemFunctionLibrary::GiveAbility( UAbilitySystemComponent * asc, TSubclassOf< UGameplayAbility > ability, int32 level, UObject * source_object )
{
    FGameplayAbilitySpec spec( ability, level, INDEX_NONE, source_object );
    return asc->GiveAbility( spec );
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

void UGASExtAbilitySystemFunctionLibrary::SendGameplayEventToASC( UAbilitySystemComponent * asc, FGameplayTag event_tag, FGameplayEventData payload )
{
    if ( IsValid( asc ) )
    {
        FScopedPredictionWindow new_scoped_window( asc, true );
        asc->HandleGameplayEvent( event_tag, &payload );
    }
    else
    {
        ABILITY_LOG( Error, TEXT( "UGASExtAbilitySystemFunctionLibrary::SendGameplayEventToASC: Invalid ability system component" ) );
    }
}

bool UGASExtAbilitySystemFunctionLibrary::DoesASCHaveAttributeSetForAttribute( UAbilitySystemComponent * asc, FGameplayAttribute attribute )
{
    if ( asc == nullptr )
    {
        return false;
    }

    return asc->HasAttributeSetForAttribute( attribute );
}

TSubclassOf< UGameplayEffect > UGASExtAbilitySystemFunctionLibrary::GetGameplayEffectClassFromSpecHandle( FGameplayEffectSpecHandle spec_handle )
{
    if ( const auto * spec = spec_handle.Data.Get() )
    {
        return spec->Def->GetClass();
    }

    return nullptr;
}

void UGASExtAbilitySystemFunctionLibrary::CopySetByCallerTagMagnitudesFromSpecToConditionalEffects( FGameplayEffectSpec * gameplay_effect_spec )
{
    if ( gameplay_effect_spec == nullptr )
    {
        return;
    }

    for ( auto handle : gameplay_effect_spec->TargetEffectSpecs )
    {
        if ( !handle.Data.IsValid() )
        {
            continue;
        }

        handle.Data->SetByCallerTagMagnitudes.Append( gameplay_effect_spec->SetByCallerTagMagnitudes );
        CopySetByCallerTagMagnitudesFromSpecToConditionalEffects( handle.Data.Get() );
    }
}

void UGASExtAbilitySystemFunctionLibrary::AddDynamicAssetTagToSpecAndChildren( FGameplayEffectSpec * gameplay_effect_spec, const FGameplayTag gameplay_tag )
{
    if ( gameplay_effect_spec == nullptr )
    {
        return;
    }

    gameplay_effect_spec->AddDynamicAssetTag( gameplay_tag );

    for ( auto handle : gameplay_effect_spec->TargetEffectSpecs )
    {
        if ( !handle.Data.IsValid() )
        {
            continue;
        }

        handle.Data->AddDynamicAssetTag( gameplay_tag );
        AddDynamicAssetTagToSpecAndChildren( handle.Data.Get(), gameplay_tag );
    }
}