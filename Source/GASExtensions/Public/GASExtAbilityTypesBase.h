#pragma once

#include <Abilities/GameplayAbilityTargetTypes.h>
#include <Abilities/GameplayAbilityTypes.h>
#include <CoreMinimal.h>
#include <Engine/CollisionProfile.h>

#include "GASExtAbilityTypesBase.generated.h"

class ASWSpline;
class UGameplayEffect;
class UGASExtTargetType;

UENUM( BlueprintType )
enum class EGASExtTargetTraceType : uint8
{
    Line,
    Sphere
};

UENUM( BlueprintType )
enum class EGASExtCollisionDetectionType : uint8
{
    UsingCollisionProfile,
    UsingCollisionChannel
};

USTRUCT( BlueprintType )
struct FGASExtCollisionDetectionInfo
{
    GENERATED_BODY()

    FGASExtCollisionDetectionInfo()
    {
        DetectionType = EGASExtCollisionDetectionType::UsingCollisionChannel;
        TraceProfile = UCollisionProfile::BlockAll_ProfileName;
        TraceChannel = ECollisionChannel::ECC_WorldStatic;
        ItUsesTraceComplex = true;
        IgnoreBlockingHits = false;
        ItReturnsPhysicalMaterial = true;
    }

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    EGASExtCollisionDetectionType DetectionType;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, meta = ( EditCondition = "DetectionType == EGASExtCollisionDetectionType::UsingCollisionProfile" ) )
    FCollisionProfileName TraceProfile;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, meta = ( EditCondition = "DetectionType == EGASExtCollisionDetectionType::UsingCollisionChannel" ) )
    TEnumAsByte< ECollisionChannel > TraceChannel;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    uint8 ItUsesTraceComplex : 1;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    uint8 IgnoreBlockingHits : 1;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    uint8 ItReturnsPhysicalMaterial : 1;
};

USTRUCT( BlueprintType )
struct FGASExtGameplayEffectContainer
{
    GENERATED_BODY()

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "GameplayEffectContainer" )
    TSubclassOf< UGASExtTargetType > TargetTypeClass;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "GameplayEffectContainer" )
    TArray< TSubclassOf< UGameplayEffect > > TargetGameplayEffectClasses;

    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayEffectContainer" )
    TArray< FGameplayTag > GameplayEventTags;
};

USTRUCT( BlueprintType )
struct FGASExtGameplayEffectContainerSpec
{
    GENERATED_BODY()

    UPROPERTY()
    FGameplayAbilityTargetDataHandle TargetData;

    UPROPERTY()
    TArray< FGameplayEffectSpecHandle > TargetGameplayEffectSpecs;

    UPROPERTY()
    TArray< FGameplayTag > GameplayEventTags;

    UPROPERTY()
    FGameplayCueParameters GameplayCueParameters;

    UPROPERTY()
    FGameplayEventData EventDataPayload;
};

USTRUCT( BlueprintType )
struct FGASExtGameplayAbilityTargetData_LocationInfo : public FGameplayAbilityTargetData_LocationInfo
{
    GENERATED_USTRUCT_BODY()

    FTransform GetEndPointTransform() const override
    {
        return TargetLocation.GetTargetingTransform();
    }

    UScriptStruct * GetScriptStruct() const override
    {
        return FGASExtGameplayAbilityTargetData_LocationInfo::StaticStruct();
    }

    FString ToString() const override
    {
        return TEXT( "FGASExtGameplayAbilityTargetData_LocationInfo" );
    }

    bool NetSerialize( FArchive & archive, class UPackageMap * package_map, bool & success );
};

template <>
struct TStructOpsTypeTraits< FGASExtGameplayAbilityTargetData_LocationInfo > : public TStructOpsTypeTraitsBase2< FGASExtGameplayAbilityTargetData_LocationInfo >
{
    enum
    {
        WithNetSerializer = true // For now this is REQUIRED for FGameplayAbilityTargetDataHandle net serialization to work
    };
};