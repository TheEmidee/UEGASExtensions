#include "Targeting/GASExtTargetDataGenerator.h"

#include <Abilities/GameplayAbilityTypes.h>
#include <AbilitySystemBlueprintLibrary.h>
#include <Kismet/KismetSystemLibrary.h>

FGameplayAbilityTargetDataHandle UGASExtTargetDataGenerator_EventData::GetTargetData( AActor * /*ability_owner*/, const FGameplayEventData & event_data ) const
{
    return FGameplayAbilityTargetDataHandle( event_data.TargetData );
}

//FGameplayAbilityTargetDataHandle UGASExtTargetDataGenerator_HitResult::GetTargetData( AActor * /*ability_owner*/, const FGameplayEventData & /*event_data*/  ) const
//{
//    auto * new_data = new FGameplayAbilityTargetData_SingleTargetHit( FHitResult() );
//    return FGameplayAbilityTargetDataHandle( new_data );
//}

FGameplayAbilityTargetDataHandle UGASExtTargetDataGenerator_GetOwner::GetTargetData( AActor * ability_owner, const FGameplayEventData & /*event_data*/  ) const
{
    auto * new_data = new FGameplayAbilityTargetData_ActorArray();
    new_data->TargetActorArray.Add( ability_owner );
    return FGameplayAbilityTargetDataHandle( new_data );
}

UGASExtTargetDataGenerator_SphereOverlapBase::UGASExtTargetDataGenerator_SphereOverlapBase()
{
    SphereRadius = 1.0f;
    ObjectTypes.Add( UEngineTypes::ConvertToObjectType( ECC_Pawn ) );
    ObjectTypes.Add( UEngineTypes::ConvertToObjectType( ECC_Destructible ) );
    bDrawsDebug = false;
    bMustHaveLineOfSight = true;
    DrawDebugDuration = 2.0f;
    SphereCenterOffset = FVector::ZeroVector;
}

FGameplayAbilityTargetDataHandle UGASExtTargetDataGenerator_SphereOverlapBase::GetTargetDataAtLocation( AActor * ability_owner, const FVector location ) const
{
    TArray< AActor * > hit_actors;

    const auto sphere_center = location + SphereCenterOffset;

    UKismetSystemLibrary::SphereOverlapActors( ability_owner, sphere_center, SphereRadius.GetValue(), ObjectTypes, nullptr, TArray< AActor * > { ability_owner }, hit_actors );

    if ( bDrawsDebug )
    {
        UKismetSystemLibrary::DrawDebugSphere( ability_owner, sphere_center, SphereRadius.GetValue(), 12, FLinearColor::Red, DrawDebugDuration, 1.0f );
    }

    if ( bMustHaveLineOfSight )
    {

        for ( auto index = hit_actors.Num() - 1; index >= 0; --index )
        {
            auto * hit_actor = hit_actors[ index ];

            auto ignore_actors = hit_actors;
            ignore_actors.RemoveSwap( hit_actor );

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
                hit_actors.RemoveAtSwap( index );
            }
        }
    }

    auto * new_data = new FGameplayAbilityTargetData_ActorArray();
    new_data->TargetActorArray.Append( hit_actors );

    return FGameplayAbilityTargetDataHandle( new_data );
}

//FGameplayAbilityTargetDataHandle UGASExtTargetDataGenerator_SphereOverlapAtHitResult::GetTargetData( AActor * ability_owner, const FGameplayEventData & event_data ) const
//{
//    return GetTargetDataAtLocation( ability_owner, ability_owner->GetActorLocation() );
//}

FGameplayAbilityTargetDataHandle UGASExtTargetDataGenerator_SphereOverlapAtAbilityOwner::GetTargetData( AActor * ability_owner, const FGameplayEventData & event_data ) const
{
    return GetTargetDataAtLocation( ability_owner, ability_owner->GetActorLocation() );
}
