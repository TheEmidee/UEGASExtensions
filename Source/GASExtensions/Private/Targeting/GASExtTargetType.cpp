#include "Targeting/GASExtTargetType.h"

#include <Abilities/GameplayAbilityTypes.h>
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
}

FGameplayAbilityTargetDataHandle UGASExtTargetType_SphereOverlapAtHitResult::GetTargetData( AActor * ability_owner, const FHitResult & /*hit_result*/, const FGameplayEventData & event_data ) const
{
    TArray< AActor * > hit_actors;

    const auto actor_location = ability_owner->GetActorLocation();

    UKismetSystemLibrary::SphereOverlapActors( ability_owner, actor_location, SphereRadius.GetValue(), ObjectTypes, nullptr, TArray< AActor * >(), hit_actors );

    if ( bMustHaveLineOfSight )
    {
        auto ignore_actors = hit_actors;
        ignore_actors.Remove( ability_owner->GetAttachParentActor() );

        for ( auto index = hit_actors.Num() - 1; index >= 0; --index )
        {
            const auto * hit_actor = hit_actors[ index ];
            if ( hit_actor == nullptr || hit_actor == ability_owner )
            {
                continue;
            }

            FHitResult line_trace_hit;
            UKismetSystemLibrary::LineTraceSingle( ability_owner,
                actor_location,
                hit_actor->GetActorLocation(),
                UEngineTypes::ConvertToTraceType( ECC_Visibility ),
                false,
                ignore_actors,
                bDrawsDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
                line_trace_hit,
                true,
                FLinearColor::Red,
                FLinearColor::Green,
                2.0f );

            if ( line_trace_hit.bBlockingHit && line_trace_hit.GetActor() != hit_actor )
            {
                hit_actors.RemoveAt( index );
            }
        }
    }

    if ( bDrawsDebug )
    {
        UKismetSystemLibrary::DrawDebugSphere( ability_owner, actor_location, SphereRadius.GetValue(), 12, FLinearColor::Red, 1.0f, 1.0f );
    }

    auto * new_data = new FGameplayAbilityTargetData_ActorArray();
    new_data->TargetActorArray.Append( hit_actors );

    return FGameplayAbilityTargetDataHandle( new_data );
}