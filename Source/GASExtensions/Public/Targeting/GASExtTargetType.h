#pragma once

#include <Abilities/GameplayAbilityTargetTypes.h>
#include <CoreMinimal.h>

#include "GASExtTargetType.generated.h"

struct FGameplayEventData;

UENUM()
enum class EGASExtTargetDataExecutionType : uint8
{
    OnEffectContextApplication,
    OnEffectContextCreation
};

UCLASS( Blueprintable, HideDropdown, meta = ( ShowWorldContextPin ) )
class GASEXTENSIONS_API UGASExtTargetType : public UObject
{
    GENERATED_BODY()

public:
    virtual FGameplayAbilityTargetDataHandle GetTargetData( AActor * /*ability_owner*/, const FHitResult & /*hit_result*/, const FGameplayEventData & /*event_data*/ ) const
    {
        return FGameplayAbilityTargetDataHandle();
    }
};

UCLASS()
class GASEXTENSIONS_API UGASExtTargetType_EventData final : public UGASExtTargetType
{
    GENERATED_BODY()

public:
    FGameplayAbilityTargetDataHandle GetTargetData( AActor * ability_owner, const FHitResult & hit_result, const FGameplayEventData & event_data ) const override;
};

UCLASS()
class GASEXTENSIONS_API UGASExtTargetType_HitResult final : public UGASExtTargetType
{
    GENERATED_BODY()

public:
    FGameplayAbilityTargetDataHandle GetTargetData( AActor * ability_owner, const FHitResult & hit_result, const FGameplayEventData & event_data ) const override;
};

UCLASS()
class GASEXTENSIONS_API UGASExtTargetType_GetOwner final : public UGASExtTargetType
{
    GENERATED_BODY()

public:
    FGameplayAbilityTargetDataHandle GetTargetData( AActor * ability_owner, const FHitResult & hit_result, const FGameplayEventData & event_data ) const override;
};

UCLASS()
class GASEXTENSIONS_API UGASExtTargetType_SphereOverlapAtHitResult final : public UGASExtTargetType
{
    GENERATED_BODY()

public:
    UGASExtTargetType_SphereOverlapAtHitResult();

    FGameplayAbilityTargetDataHandle GetTargetData( AActor * ability_owner, const FHitResult & hit_result, const FGameplayEventData & event_data ) const override;

    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    FScalableFloat SphereRadius;

    UPROPERTY( EditAnywhere )
    TArray< TEnumAsByte< EObjectTypeQuery > > ObjectTypes;

    UPROPERTY( EditAnywhere )
    uint8 bMustHaveLineOfSight : 1;

    UPROPERTY( EditAnywhere )
    uint8 bDrawsDebug : 1;
};