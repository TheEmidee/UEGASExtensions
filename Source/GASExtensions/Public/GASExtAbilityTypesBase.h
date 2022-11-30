#pragma once

#include "Targeting/GASExtTargetType.h"

#include <Abilities/GameplayAbilityTargetTypes.h>
#include <Abilities/GameplayAbilityTypes.h>
#include <CoreMinimal.h>
#include <Engine/CollisionProfile.h>

#include "GASExtAbilityTypesBase.generated.h"

class UGASExtFallOffType;
class ASWSpline;
class UGameplayEffect;
class UGASExtTargetType;

UENUM( BlueprintType )
enum class EGASExtAbilityActivationPolicy : uint8
{
    // Try to activate the ability when the input is triggered.
    OnInputTriggered,

    // Continually try to activate the ability while the input is active.
    WhileInputActive,

    // Try to activate the ability when an avatar is assigned.
    OnSpawn
};

UENUM( BlueprintType )
enum class EGASExtAbilityActivationGroup : uint8
{
    // Ability runs independently of all other abilities.
    Independent,

    // Ability is canceled and replaced by other exclusive abilities.
    ExclusiveReplaceable,

    // Ability blocks all other exclusive abilities from activating.
    ExclusiveBlocking,

    MAX UMETA( Hidden )
};

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
struct GASEXTENSIONS_API FGASExtCollisionDetectionInfo
{
    GENERATED_BODY()

    FGASExtCollisionDetectionInfo()
    {
        DetectionType = EGASExtCollisionDetectionType::UsingCollisionChannel;
        TraceProfile = UCollisionProfile::BlockAll_ProfileName;
        TraceChannel = ECollisionChannel::ECC_WorldStatic;
        bUsesTraceComplex = true;
        bIgnoreBlockingHits = false;
        bReturnsPhysicalMaterial = true;
    }

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    EGASExtCollisionDetectionType DetectionType;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, meta = ( EditCondition = "DetectionType == EGASExtCollisionDetectionType::UsingCollisionProfile" ) )
    FCollisionProfileName TraceProfile;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, meta = ( EditCondition = "DetectionType == EGASExtCollisionDetectionType::UsingCollisionChannel" ) )
    TEnumAsByte< ECollisionChannel > TraceChannel;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    uint8 bUsesTraceComplex : 1;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    uint8 bIgnoreBlockingHits : 1;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    uint8 bReturnsPhysicalMaterial : 1;
};

USTRUCT( BlueprintType )
struct GASEXTENSIONS_API FGASExtGameplayEffectContainer
{
    GENERATED_BODY()

    FGASExtGameplayEffectContainer();

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Instanced, Category = "GameplayEffectContainer" )
    UGASExtFallOffType * FallOffType;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Instanced, Category = "GameplayEffectContainer" )
    UGASExtTargetType * TargetType;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "GameplayEffectContainer", meta = ( EditCondition = "TargetType != nullptr", EditConditionHides ) )
    EGASExtGetTargetDataExecutionType TargetDataExecutionType;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "GameplayEffectContainer" )
    TMap< FGameplayTag, FScalableFloat > SetByCallerTagsToMagnitudeMap;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "GameplayEffectContainer" )
    TArray< TSubclassOf< UGameplayEffect > > TargetGameplayEffectClasses;

    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayEffectContainer" )
    TArray< FGameplayTag > GameplayEventTags;
};

USTRUCT( BlueprintType )
struct GASEXTENSIONS_API FGASExtGameplayEffectContainerSpec
{
    GENERATED_BODY()

    UPROPERTY( BlueprintReadOnly, Category = "GameplayEffectContainerSpec" )
    FGameplayAbilityTargetDataHandle TargetData;

    UPROPERTY( BlueprintReadOnly, Category = "GameplayEffectContainerSpec" )
    TArray< FGameplayEffectSpecHandle > TargetGameplayEffectSpecHandles;

    UPROPERTY( BlueprintReadOnly, Category = "GameplayEffectContainerSpec" )
    TArray< FGameplayTag > GameplayEventTags;

    UPROPERTY( BlueprintReadOnly, Category = "GameplayEffectContainerSpec" )
    FGameplayCueParameters GameplayCueParameters;

    UPROPERTY( BlueprintReadOnly, Category = "GameplayEffectContainerSpec" )
    FGameplayEventData EventDataPayload;

    UPROPERTY( BlueprintReadOnly, Category = "GameplayEffectContainerSpec" )
    EGASExtGetTargetDataExecutionType TargetDataExecutionType;
};

USTRUCT( BlueprintType )
struct GASEXTENSIONS_API FGASExtGameplayEffectContext : public FGameplayEffectContext
{
    GENERATED_BODY()

public:
    FGASExtGameplayEffectContext();

    UScriptStruct * GetScriptStruct() const override;
    FGameplayEffectContext * Duplicate() const override;
    bool NetSerialize( FArchive & ar, UPackageMap * map, bool & out_success ) override;

    UGASExtFallOffType * GetFallOffType() const;
    void SetFallOffType( UGASExtFallOffType * fall_off_type );

    UGASExtTargetType * GetTargetType() const;
    void SetTargetType( UGASExtTargetType * target_type );

protected:
    UPROPERTY()
    UGASExtFallOffType * FallOffType;

    UPROPERTY()
    UGASExtTargetType * TargetType;
};

FORCEINLINE UGASExtFallOffType * FGASExtGameplayEffectContext::GetFallOffType() const
{
    return FallOffType;
}

FORCEINLINE UGASExtTargetType * FGASExtGameplayEffectContext::GetTargetType() const
{
    return TargetType;
}

template <>
struct TStructOpsTypeTraits< FGASExtGameplayEffectContext > : public TStructOpsTypeTraitsBase2< FGASExtGameplayEffectContext >
{
    enum
    {
        WithNetSerializer = true,
        WithCopy = true // Necessary so that TSharedPtr<FHitResult> Data is copied around
    };
};

USTRUCT( BlueprintType )
struct GASEXTENSIONS_API FGASExtGameplayAbilityTargetData_LocationInfo : public FGameplayAbilityTargetData_LocationInfo
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