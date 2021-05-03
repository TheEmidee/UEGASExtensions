#pragma once

#include "GASExtAbilityTypesBase.h"

#include <Abilities/GameplayAbilityTargetDataFilter.h>
#include <Abilities/GameplayAbilityTargetTypes.h>
#include <CollisionQueryParams.h>
#include <CoreMinimal.h>
#include <Kismet/BlueprintFunctionLibrary.h>

#include "GASExtTargetingHelperLibrary.generated.h"

struct FGASExtCollisionDetectionInfo;

USTRUCT( BlueprintType )
struct GASEXTENSIONS_API FSWTargetingLocationInfo
{
    GENERATED_USTRUCT_BODY()

public:
    FSWTargetingLocationInfo() = default;

    FGameplayAbilityTargetingLocationInfo Source;

    FGameplayAbilityTargetingLocationInfo Target;
};

struct FSWAimWithPlayerControllerInfos
{
    FSWAimWithPlayerControllerInfos( const UGameplayAbility * ability, const FVector & trace_start, const FCollisionQueryParams & collision_query_params, const FGameplayAbilityTargetingLocationInfo & start_location_infos, const FGASExtCollisionDetectionInfo & collision_info, const FGameplayTargetDataFilterHandle & target_data_filter_handle, const float max_range, const bool ignore_pitch = false ) :
        Ability( ability ),
        TraceStart( trace_start ),
        CollisionQueryParams( collision_query_params ),
        StartLocationInfos( start_location_infos ),
        CollisionInfo( collision_info ),
        TargetDataFilterHandle( target_data_filter_handle ),
        MaxRange( max_range ),
        IgnorePitch( ignore_pitch )
    {
    }

    const UGameplayAbility * Ability;
    const FVector & TraceStart;
    const FCollisionQueryParams & CollisionQueryParams;
    const FGameplayAbilityTargetingLocationInfo & StartLocationInfos;
    const FGASExtCollisionDetectionInfo & CollisionInfo;
    const FGameplayTargetDataFilterHandle & TargetDataFilterHandle;
    const float MaxRange;
    const bool IgnorePitch;
};

struct FSWAimFromComponentInfos
{
    FSWAimFromComponentInfos( const UGameplayAbility * ability, const FVector & trace_start, const FCollisionQueryParams & collision_query_params, const FGameplayAbilityTargetingLocationInfo & start_location_infos, const FGASExtCollisionDetectionInfo & collision_info, const FGameplayTargetDataFilterHandle & target_data_filter_handle, const float max_range ) :
        Ability( ability ),
        TraceStart( trace_start ),
        CollisionQueryParams( collision_query_params ),
        StartLocationInfos( start_location_infos ),
        CollisionInfo( collision_info ),
        TargetDataFilterHandle( target_data_filter_handle ),
        MaxRange( max_range )
    {
    }

    const UGameplayAbility * Ability;
    const FVector & TraceStart;
    const FCollisionQueryParams & CollisionQueryParams;
    const FGameplayAbilityTargetingLocationInfo & StartLocationInfos;
    const FGASExtCollisionDetectionInfo & CollisionInfo;
    const FGameplayTargetDataFilterHandle & TargetDataFilterHandle;
    const float MaxRange;
};

struct FSWSpreadInfos
{
    FSWSpreadInfos( const FVector & trace_start, const float targeting_spread, const float max_range ) :
        TraceStart( trace_start ),
        TargetingSpread( targeting_spread ),
        MaxRange( max_range )
    {
    }

    const FVector & TraceStart;
    const float TargetingSpread;
    const float MaxRange;
};

struct FGameplayTargetDataFilterHandle;

UCLASS()
class GASEXTENSIONS_API UGASExtTargetingHelperLibrary final : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    static void LineTraceWithFilter( TArray< FHitResult > & hit_results, UWorld * world, const FGameplayTargetDataFilterHandle & target_data_filter_handle, const FVector & trace_start, const FVector & trace_end, const FGASExtCollisionDetectionInfo & collision_info, const FCollisionQueryParams & collision_query_params );
    static void SphereTraceWithFilter( TArray< FHitResult > & hit_results, UWorld * world, const FGameplayTargetDataFilterHandle & target_data_filter_handle, const FVector & trace_start, const FVector & trace_end, float sphere_radius, const FGASExtCollisionDetectionInfo & collision_info, const FCollisionQueryParams & collision_query_params );

    UFUNCTION( BlueprintCallable )
    static void FilterHitResults( TArray< FHitResult > & hit_results, const FVector & trace_start, const FVector & trace_end, const FGameplayTargetDataFilterHandle & target_data_filter_handle );

    UFUNCTION( BlueprintCallable )
    static FGameplayAbilityTargetDataHandle MakeTargetDataFromHitResults( const TArray< FHitResult > & hit_results );

    UFUNCTION( BlueprintCallable )
    static FGameplayAbilityTargetDataHandle MakeTargetDataFromLocationInfos( const FGameplayAbilityTargetingLocationInfo & source, const FGameplayAbilityTargetingLocationInfo & target );

    UFUNCTION( BlueprintCallable )
    static FGameplayAbilityTargetDataHandle MakeTargetDataFromMultipleLocationInfos( const TArray< FSWTargetingLocationInfo > & location_infos );

    static void AimWithPlayerController( FVector & trace_end, const FSWAimWithPlayerControllerInfos & aim_infos );
    static void AimFromComponent( FVector & trace_end, const FSWAimFromComponentInfos & aim_infos );
    static void ComputeTraceEndWithSpread( FVector & trace_end, const FSWSpreadInfos & spread_infos );
};