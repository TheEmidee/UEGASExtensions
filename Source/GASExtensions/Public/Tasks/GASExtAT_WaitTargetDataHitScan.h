#pragma once

#include "GASExtAT_WaitTargetData.h"
#include "GASExtAbilityTypesBase.h"

#include <Abilities/GameplayAbilityTargetDataFilter.h>
#include <CoreMinimal.h>
#include <Kismet/KismetSystemLibrary.h>

#include "GASExtAT_WaitTargetDataHitScan.generated.h"

USTRUCT( BlueprintType )
struct GASEXTENSIONS_API FGASExtWaitTargetDataHitScanOptions
{
    GENERATED_USTRUCT_BODY()

    FGASExtWaitTargetDataHitScanOptions();

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    uint8 bAimFromPlayerViewPoint : 1;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    EGASExtTargetTraceType TargetTraceType;

    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly )
    FGASExtCollisionDetectionInfo CollisionInfo;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    FGameplayTargetDataFilterHandle TargetDataFilterHandle;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    FScalableFloat MaxRange;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    FScalableFloat NumberOfTraces;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    uint8 bMaxHitResultsPerTrace;

    // In degrees
    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    FScalableFloat TargetingSpread;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    uint8 bTraceAffectsAimPitch : 1;

    /* If greater than zero, will generate a sphere cast. Otherwise, a line cast */
    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, meta = ( EditCondition = "TargetTraceType == EGASExtTargetTraceType::Sphere" ) )
    float TraceSphereRadius;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    uint8 bShowDebugTraces : 1;
};

UCLASS()
class GASEXTENSIONS_API UGASExtAT_WaitTargetDataHitScan : public UGASExtAT_WaitTargetData
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable, meta = ( HidePin = "owning_ability", DefaultToSelf = "owning_ability", BlueprintInternalUseOnly = "true", HideSpawnParms = "Instigator" ), Category = "Ability|Tasks" )
    static UGASExtAT_WaitTargetDataHitScan * WaitTargetDataHitScan(
        UGameplayAbility * owning_ability,
        FName task_instance_name,
        const FGameplayAbilityTargetingLocationInfo & start_trace_location_infos,
        const FGASExtWaitTargetDataReplicationOptions & replication_options,
        const FGASExtWaitTargetDataHitScanOptions & hit_scan_options );

private:
    void DoTrace( TArray< FHitResult > & hit_results, UWorld * world, const FGameplayTargetDataFilterHandle & target_data_filter_handle, const FVector & trace_start, const FVector & trace_end, const FGASExtCollisionDetectionInfo & collision_info, const FCollisionQueryParams & collision_query_params ) const;
    void ShowDebugTraces( const TArray< FHitResult > & hit_results, const FVector & trace_start, const FVector & trace_end, EDrawDebugTrace::Type draw_debug_type, float duration = 2.0f ) const;
    FGameplayAbilityTargetDataHandle ProduceTargetData() override;
    TArray< FHitResult > PerformTrace() const;

    FGASExtWaitTargetDataHitScanOptions Options;
    FGameplayAbilityTargetingLocationInfo StartLocationInfo;
};
