#include "Tasks/GASExtAT_WaitTargetDataHitScan.h"

#include "Targeting/GASExtTargetingHelperLibrary.h"

#include <AbilitySystemComponent.h>
#include <DrawDebugHelpers.h>
#include <GameFramework/PlayerController.h>

FGASExtWaitTargetDataHitScanOptions::FGASExtWaitTargetDataHitScanOptions()
{
    bAimFromPlayerViewPoint = true;
    TargetTraceType = EGASExtTargetTraceType::Line;
    MaxRange.Value = 999999.0f;
    NumberOfTraces.Value = 1;
    bMaxHitResultsPerTrace = 1;
    TargetingSpread.Value = 0.0f;
    bTraceAffectsAimPitch = true;
    bShowDebugTraces = false;
    TraceSphereRadius = 10.0f;
}

UGASExtAT_WaitTargetDataHitScan * UGASExtAT_WaitTargetDataHitScan::WaitTargetDataHitScan( UGameplayAbility * owning_ability, FName task_instance_name, const FGameplayAbilityTargetingLocationInfo & start_trace_location_infos, const FGASExtWaitTargetDataReplicationOptions & replication_options, const FGASExtWaitTargetDataHitScanOptions & hit_scan_options )
{
    auto * my_obj = NewAbilityTask< UGASExtAT_WaitTargetDataHitScan >( owning_ability, task_instance_name );
    my_obj->ReplicationOptions = replication_options;
    my_obj->Options = hit_scan_options;
    my_obj->StartLocationInfo = start_trace_location_infos;
    return my_obj;
}

void UGASExtAT_WaitTargetDataHitScan::DoTrace( TArray< FHitResult > & hit_results, UWorld * world, const FGameplayTargetDataFilterHandle & target_data_filter_handle, const FVector & trace_start, const FVector & trace_end, const FGASExtCollisionDetectionInfo & collision_info, const FCollisionQueryParams & collision_query_params ) const
{
    if ( Options.TargetTraceType == EGASExtTargetTraceType::Sphere )
    {
        UGASExtTargetingHelperLibrary::SphereTraceWithFilter( hit_results, world, target_data_filter_handle, trace_start, trace_end, Options.TraceSphereRadius, collision_info, collision_query_params );
    }
    else
    {
        UGASExtTargetingHelperLibrary::LineTraceWithFilter( hit_results, world, target_data_filter_handle, trace_start, trace_end, collision_info, collision_query_params );
    }
}

void UGASExtAT_WaitTargetDataHitScan::ShowDebugTraces( const TArray< FHitResult > & hit_results, const FVector & trace_start, const FVector & trace_end, EDrawDebugTrace::Type draw_debug_type, float duration ) const
{
#if ENABLE_DRAW_DEBUG
    if ( Options.TargetTraceType == EGASExtTargetTraceType::Sphere )
    {
        //UGBFTraceBlueprintLibrary::DrawDebugSphereTraceMulti( GetWorld(), trace_start, trace_end, Options.TraceSphereRadius, draw_debug_type, true, hit_results, FLinearColor::Green, FLinearColor::Red, duration );
    }
    else
    {
        //UGBFTraceBlueprintLibrary::DrawDebugLineTraceMulti( GetWorld(), trace_start, trace_end, draw_debug_type, true, hit_results, FLinearColor::Green, FLinearColor::Red, duration );
    }
#endif
}

FGameplayAbilityTargetDataHandle UGASExtAT_WaitTargetDataHitScan::ProduceTargetData()
{
    check( IsValid( Ability ) );

    const auto hit_results = PerformTrace();
    const auto target_data_handle = UGASExtTargetingHelperLibrary::MakeTargetDataFromHitResults( hit_results );

    return target_data_handle;
}

TArray< FHitResult > UGASExtAT_WaitTargetDataHitScan::PerformTrace() const
{
    const auto actor_info = Ability->GetCurrentActorInfo();

    TArray< AActor * > actors_to_ignore;

    actors_to_ignore.Add( actor_info->AvatarActor.Get() );

    FCollisionQueryParams collision_query_params( SCENE_QUERY_STAT( USWAT_WaitTargetDataHitScan ), Options.CollisionInfo.bUsesTraceComplex );
    collision_query_params.bReturnPhysicalMaterial = Options.CollisionInfo.bReturnsPhysicalMaterial;
    collision_query_params.AddIgnoredActors( actors_to_ignore );
    collision_query_params.bIgnoreBlocks = Options.CollisionInfo.bIgnoreBlockingHits;

    auto trace_start = StartLocationInfo.GetTargetingTransform().GetLocation();
    FVector trace_end;

    auto trace_from_player_view_point = Options.bAimFromPlayerViewPoint;
    if ( trace_from_player_view_point )
    {
        // :TODO: Fix aiming for AI
        if ( auto * pc = Ability->GetCurrentActorInfo()->PlayerController.Get() )
        {
            FRotator view_rotation;
            pc->GetPlayerViewPoint( trace_start, view_rotation );
        }
        else
        {
            trace_from_player_view_point = false;
        }
    }

    TArray< FHitResult > returned_hit_results;

    const auto world = Ability->GetCurrentActorInfo()->OwnerActor->GetWorld();

    for ( auto number_of_trace_index = 0; number_of_trace_index < Options.NumberOfTraces.GetValue(); number_of_trace_index++ )
    {
        if ( trace_from_player_view_point )
        {
            UGASExtTargetingHelperLibrary::AimWithPlayerController( trace_end,
                FSWAimWithPlayerControllerInfos(
                    Ability,
                    trace_start,
                    collision_query_params,
                    StartLocationInfo,
                    Options.CollisionInfo,
                    Options.TargetDataFilterHandle,
                    Options.MaxRange.GetValue() ) );
        }
        else
        {
            UGASExtTargetingHelperLibrary::AimFromComponent(
                trace_end,
                FSWAimFromComponentInfos(
                    Ability,
                    trace_start,
                    collision_query_params,
                    StartLocationInfo,
                    Options.CollisionInfo,
                    Options.TargetDataFilterHandle,
                    Options.MaxRange.GetValue() ) );
        }

        UGASExtTargetingHelperLibrary::ComputeTraceEndWithSpread(
            trace_end,
            FSWSpreadInfos(
                trace_start,
                Options.TargetingSpread.GetValue(),
                Options.MaxRange.GetValue() ) );

        TArray< FHitResult > trace_hit_results;
        DoTrace( trace_hit_results, world, Options.TargetDataFilterHandle, trace_start, trace_end, Options.CollisionInfo, collision_query_params );

        if ( Options.bMaxHitResultsPerTrace >= 0 && trace_hit_results.Num() + 1 > Options.bMaxHitResultsPerTrace )
        {
            // Trim to MaxHitResults
            trace_hit_results.SetNum( Options.bMaxHitResultsPerTrace );
        }

        if ( trace_hit_results.Num() < 1 )
        {
            // If there were no hits, add a default HitResult at the end of the trace
            // :TODO: Keep ?
            // :Mike: This ensures the target data event is still broadcasted?
            FHitResult hit_result;
            // Start param could be player ViewPoint. We want HitResult to always display the StartLocation.
            hit_result.TraceStart = StartLocationInfo.GetTargetingTransform().GetLocation();
            hit_result.TraceEnd = trace_end;
            hit_result.Location = trace_end;
            hit_result.ImpactPoint = trace_end;
            trace_hit_results.Add( hit_result );
        }

#if ENABLE_DRAW_DEBUG
        if ( Options.bShowDebugTraces )
        {
            ShowDebugTraces( trace_hit_results, trace_start, trace_end, EDrawDebugTrace::Type::ForDuration, 2.0f );
        }
#endif

        returned_hit_results.Append( trace_hit_results );
    }

    // Start param could be player ViewPoint. We want HitResult to always display the StartLocation.
    const auto corrected_trace_start = StartLocationInfo.GetTargetingTransform().GetLocation();
    UGASExtTargetingHelperLibrary::FilterHitResults( returned_hit_results, corrected_trace_start, trace_end, Options.TargetDataFilterHandle );

    return returned_hit_results;
}
