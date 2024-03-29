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

struct FSWAimInfos
{
    FSWAimInfos( const UGameplayAbility * ability, const FGameplayAbilityTargetingLocationInfo & start_location_infos, const float max_range, const FVector & location_offset = FVector::ZeroVector, const FRotator & rotation_offset = FRotator::ZeroRotator ) :
        Ability( ability ),
        StartLocationInfos( start_location_infos ),
        MaxRange( max_range ),
        LocationOffset( location_offset ),
        RotationOffset( rotation_offset )
    {
    }

    const UGameplayAbility * Ability;
    const FGameplayAbilityTargetingLocationInfo & StartLocationInfos;
    const float MaxRange;
    const FVector LocationOffset;
    const FRotator RotationOffset;
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
    static void BoxTraceWithFilter( TArray< FHitResult > & hit_results, UWorld * world, const FGameplayTargetDataFilterHandle & target_data_filter_handle, const FVector & trace_start, const FVector & trace_end, const FVector & box_half_extent, const FGASExtCollisionDetectionInfo & collision_info, const FCollisionQueryParams & collision_query_params );

    UFUNCTION( BlueprintCallable )
    static void FilterHitResults( TArray< FHitResult > & hit_results, const FVector & trace_start, const FVector & trace_end, const FGameplayTargetDataFilterHandle & target_data_filter_handle );

    UFUNCTION( BlueprintCallable )
    static FGameplayAbilityTargetDataHandle MakeTargetDataFromHitResults( const TArray< FHitResult > & hit_results );

    UFUNCTION( BlueprintCallable )
    static FGameplayAbilityTargetDataHandle MakeTargetDataFromLocationInfos( const FGameplayAbilityTargetingLocationInfo & source, const FGameplayAbilityTargetingLocationInfo & target );

    UFUNCTION( BlueprintCallable )
    static FGameplayAbilityTargetDataHandle MakeTargetDataFromMultipleLocationInfos( const TArray< FSWTargetingLocationInfo > & location_infos );

    static void AimWithPlayerController( FVector & trace_start, FVector & trace_end, const FSWAimInfos & aim_infos );
    static void AimFromStartLocation( FVector & trace_start, FVector & trace_end, const FSWAimInfos & aim_infos );
    static void ComputeTraceEndWithSpread( FVector & trace_end, const FSWSpreadInfos & spread_infos );

private:
    static void ShapeTraceWithFilter( TArray< FHitResult > & hit_results, const UWorld * world, const FGameplayTargetDataFilterHandle & target_data_filter_handle, const FVector & trace_start, const FVector & trace_end, const FGASExtCollisionDetectionInfo & collision_info, const FCollisionQueryParams & collision_query_params, const FCollisionShape & collision_shape );


};