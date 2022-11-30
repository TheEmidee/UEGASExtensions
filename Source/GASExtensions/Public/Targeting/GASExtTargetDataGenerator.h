#pragma once

#include <Abilities/GameplayAbilityTargetTypes.h>
#include <CoreMinimal.h>

#include "GASExtTargetDataGenerator.generated.h"

struct FGameplayEventData;

UENUM()
enum class EGASExtTargetDataGenerationPhase : uint8
{
    OnEffectContextApplication,
    OnEffectContextCreation
};

UCLASS( NotBlueprintable, EditInlineNew, HideDropdown, meta = ( ShowWorldContextPin ) )
class GASEXTENSIONS_API UGASExtTargetDataGenerator : public UObject
{
    GENERATED_BODY()

public:
    virtual FGameplayAbilityTargetDataHandle GetTargetData( AActor * /*ability_owner*/, const FGameplayEventData & /*event_data*/  ) const
    {
        return FGameplayAbilityTargetDataHandle();
    }
};

UCLASS()
class GASEXTENSIONS_API UGASExtTargetDataGenerator_EventData final : public UGASExtTargetDataGenerator
{
    GENERATED_BODY()

public:
    FGameplayAbilityTargetDataHandle GetTargetData( AActor * ability_owner, const FGameplayEventData & event_data ) const override;
};

//UCLASS()
//class GASEXTENSIONS_API UGASExtTargetDataGenerator_HitResult final : public UGASExtTargetDataGenerator
//{
//    GENERATED_BODY()
//
//public:
//    FGameplayAbilityTargetDataHandle GetTargetData( AActor * ability_owner, const FGameplayEventData & event_data ) const override;
//};

UCLASS()
class GASEXTENSIONS_API UGASExtTargetDataGenerator_GetOwner final : public UGASExtTargetDataGenerator
{
    GENERATED_BODY()

public:
    FGameplayAbilityTargetDataHandle GetTargetData( AActor * ability_owner, const FGameplayEventData & event_data ) const override;
};

UCLASS( Abstract )
class GASEXTENSIONS_API UGASExtTargetDataGenerator_SphereOverlapBase : public UGASExtTargetDataGenerator
{
    GENERATED_BODY()

public:
    UGASExtTargetDataGenerator_SphereOverlapBase();

    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    FScalableFloat SphereRadius;

    UPROPERTY( EditAnywhere )
    FVector SphereCenterOffset;

    UPROPERTY( EditAnywhere )
    TArray< TEnumAsByte< EObjectTypeQuery > > ObjectTypes;

    UPROPERTY( EditAnywhere )
    uint8 bMustHaveLineOfSight : 1;

    UPROPERTY( EditAnywhere )
    uint8 bDrawsDebug : 1;

    UPROPERTY( EditAnywhere, meta = ( EditCondition = "bDrawsDebug" ) )
    float DrawDebugDuration;

protected:
    FGameplayAbilityTargetDataHandle GetTargetDataAtLocation( AActor * ability_owner, const FVector location ) const;
};

//UCLASS()
//class GASEXTENSIONS_API UGASExtTargetDataGenerator_SphereOverlapAtHitResult final : public UGASExtTargetDataGenerator_SphereOverlapBase
//{
//    GENERATED_BODY()
//
//public:
//    FGameplayAbilityTargetDataHandle GetTargetData( AActor * ability_owner, const FGameplayEventData & event_data ) const override;
//};

UCLASS()
class GASEXTENSIONS_API UGASExtTargetDataGenerator_SphereOverlapAtAbilityOwner final : public UGASExtTargetDataGenerator_SphereOverlapBase
{
    GENERATED_BODY()

public:
    FGameplayAbilityTargetDataHandle GetTargetData( AActor * ability_owner, const FGameplayEventData & event_data ) const override;
};