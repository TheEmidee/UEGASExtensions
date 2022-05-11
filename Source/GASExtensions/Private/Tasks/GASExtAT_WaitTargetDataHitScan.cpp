#include "Tasks/GASExtAT_WaitTargetDataHitScan.h"

#include "BlueprintLibraries/CoreExtTraceBlueprintLibrary.h"
#include "Targeting/GASExtTargetingHelperLibrary.h"

#include <DrawDebugHelpers.h>
#include <GameFramework/PlayerController.h>

FGASExtWaitTargetDataHitScanOptions::FGASExtWaitTargetDataHitScanOptions() :
    bAimFromPlayerViewPoint( true ),
    TargetTraceType( EGASExtTargetTraceType::Line ),
    MaxHitResultsPerTrace( 1 ),
    bSpreadTraces( true ),
    bTraceAffectsAimPitch( true ),
    TraceSphereRadius( 10.0f ),
    bShowDebugTraces( false )
{
    MaxRange.Value = 999999.0f;
    NumberOfTraces.Value = 1;
    TargetingSpread.Value = 0.0f;
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
        UCoreExtTraceBlueprintLibrary::DrawDebugSphereTraceMulti( GetWorld(), trace_start, trace_end, Options.TraceSphereRadius, draw_debug_type, true, hit_results, FLinearColor::Green, FLinearColor::Red, duration );
    }
    else
    {
        UCoreExtTraceBlueprintLibrary::DrawDebugLineTraceMulti( GetWorld(), trace_start, trace_end, draw_debug_type, true, hit_results, FLinearColor::Green, FLinearColor::Red, duration );
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

    FVector trace_start;
    FVector trace_end;

    auto trace_from_player_view_point = Options.bAimFromPlayerViewPoint && actor_info->PlayerController.IsValid();

    auto aim_infos = FSWAimInfos( Ability, StartLocationInfo, Options.MaxRange.GetValue() );

    FVector calculated_trace_start;
    FVector calculated_trace_end;

    if ( trace_from_player_view_point )
    {
        UGASExtTargetingHelperLibrary::AimWithPlayerController( calculated_trace_start, calculated_trace_end, aim_infos );
    }
    else
    {
        UGASExtTargetingHelperLibrary::AimFromComponent( calculated_trace_start, calculated_trace_end, aim_infos );
    }

    trace_start = calculated_trace_start;
    trace_end = calculated_trace_end;

    const auto world = actor_info->OwnerActor->GetWorld();
    TArray< FHitResult > returned_hit_results;

    for ( auto number_of_trace_index = 0; number_of_trace_index < Options.NumberOfTraces.GetValue(); number_of_trace_index++ )
    {
        if ( Options.bSpreadTraces )
        {
            trace_end = calculated_trace_end;

            // TODO MAKE SURE WE RESET THE TRACE END!!!!!!
            UGASExtTargetingHelperLibrary::ComputeTraceEndWithSpread(
                trace_end,
                FSWSpreadInfos(
                    trace_start,
                    Options.TargetingSpread.GetValue(),
                    Options.MaxRange.GetValue() ) );
        }

        TArray< FHitResult > trace_hit_results;
        DoTrace( trace_hit_results, world, Options.TargetDataFilterHandle, trace_start, trace_end, Options.CollisionInfo, collision_query_params );

        if ( Options.MaxHitResultsPerTrace > 0 && trace_hit_results.Num() + 1 > Options.MaxHitResultsPerTrace )
        {
            // Trim to MaxHitResults
            trace_hit_results.SetNum( Options.MaxHitResultsPerTrace );
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
