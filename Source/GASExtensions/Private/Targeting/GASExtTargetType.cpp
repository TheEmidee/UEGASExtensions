#include "Targeting/GASExtTargetType.h"

#include <Abilities/GameplayAbilityTypes.h>
#include <AbilitySystemBlueprintLibrary.h>
#include <Kismet/KismetSystemLibrary.h>

FGameplayAbilityTargetDataHandle UGASExtTargetType_EventData::GetTargetData( AActor * /*ability_owner*/, const FHitResult & /*hit_result*/, const FGameplayEventData & event_data ) const
{
    return FGameplayAbilityTargetDataHandle( event_data.TargetData );
}

FGameplayAbilityTargetDataHandle UGASExtTargetType_HitResult::GetTargetData( AActor * /*ability_owner*/, const FHitResult & hit_result, const FGameplayEventData & /*event_data*/ ) const
{
    auto * new_data = new FGameplayAbilityTargetData_SingleTargetHit( hit_result );
    return FGameplayAbilityTargetDataHandle( new_data );
}

FGameplayAbilityTargetDataHandle UGASExtTargetType_GetOwner::GetTargetData( AActor * ability_owner, const FHitResult & /*hit_result*/, const FGameplayEventData & /*event_data*/ ) const
{
    auto * new_data = new FGameplayAbilityTargetData_ActorArray();
    new_data->TargetActorArray.Add( ability_owner );
    return FGameplayAbilityTargetDataHandle( new_data );
}

UGASExtTargetType_SphereOverlapAtHitResult::UGASExtTargetType_SphereOverlapAtHitResult()
{
    SphereRadius = 1.0f;
    ObjectTypes.Add( UEngineTypes::ConvertToObjectType( ECC_Pawn ) );
    ObjectTypes.Add( UEngineTypes::ConvertToObjectType( ECC_Destructible ) );
    bDrawsDebug = false;
    bMustHaveLineOfSight = true;
    DrawDebugDuration = 2.0f;
    SphereCenterOffset = FVector::ZeroVector;
}

FGameplayAbilityTargetDataHandle UGASExtTargetType_SphereOverlapAtHitResult::GetTargetData( AActor * ability_owner, const FHitResult & /*hit_result*/, const FGameplayEventData & event_data ) const
{
    TArray< AActor * > hit_actors;

    const auto sphere_center = ability_owner->GetActorLocation() + SphereCenterOffset;

    UKismetSystemLibrary::SphereOverlapActors( ability_owner, sphere_center, SphereRadius.GetValue(), ObjectTypes, nullptr, TArray< AActor * > { ability_owner }, hit_actors );

    if ( bDrawsDebug )
    {
        UKismetSystemLibrary::DrawDebugSphere( ability_owner, sphere_center, SphereRadius.GetValue(), 12, FLinearColor::Red, DrawDebugDuration, 1.0f );
    }

    if ( bMustHaveLineOfSight )
    {
        const auto ignore_actors = hit_actors;

        for ( auto index = hit_actors.Num() - 1; index >= 0; --index )
        {
            const auto * hit_actor = hit_actors[ index ];

            FHitResult line_trace_hit;
            UKismetSystemLibrary::LineTraceSingle( ability_owner,
                sphere_center,
                hit_actor->GetActorLocation(),
                UEngineTypes::ConvertToTraceType( ECC_Visibility ),
                false,
                ignore_actors,
                bDrawsDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
                line_trace_hit,
                true,
                FLinearColor::Red,
                FLinearColor::Green,
                DrawDebugDuration );

            if ( line_trace_hit.bBlockingHit && line_trace_hit.GetActor() != hit_actor )
            {
                hit_actors.RemoveAt( index );
            }
        }
    }

    auto * new_data = new FGameplayAbilityTargetData_ActorArray();
    new_data->TargetActorArray.Append( hit_actors );

    return FGameplayAbilityTargetDataHandle( new_data );
}