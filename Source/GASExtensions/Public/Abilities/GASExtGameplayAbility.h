#pragma once

#include "GASExtAbilitySystemFunctionLibrary.h"
#include "GASExtAbilityTypesBase.h"

#include <Abilities/GameplayAbility.h>
#include <CoreMinimal.h>

#include "GASExtGameplayAbility.generated.h"

class UGASExtAbilitySystemComponent;
class ASWCharacterPlayerBase;
class ASWCharacterBase;

USTRUCT()
struct GASEXTENSIONS_API FGASExtAbilityMeshMontage
{
    GENERATED_BODY()

    FGASExtAbilityMeshMontage() :
        Mesh( nullptr ),
        Montage( nullptr )
    {
    }

    FGASExtAbilityMeshMontage( USkeletalMeshComponent * mesh, UAnimMontage * montage ) :
        Mesh( mesh ),
        Montage( montage )
    {
    }

    UPROPERTY()
    USkeletalMeshComponent * Mesh;

    UPROPERTY()
    UAnimMontage * Montage;
};

UCLASS()
class GASEXTENSIONS_API UGASExtGameplayAbility : public UGameplayAbility
{
    GENERATED_BODY()

public:
    UGASExtGameplayAbility();

    EGASExtAbilityActivationGroup GetActivationGroup() const;

    UFUNCTION( BlueprintPure, DisplayName = "IsLocallyControlled" )
    bool K2_IsLocallyControlled() const;

    UFUNCTION( BlueprintPure )
    UGASExtAbilitySystemComponent * GetGASExtAbilitySystemComponent() const;

    // If an ability is marked as 'bActivateAbilityOnGranted', activate them immediately when given here
    // Epic's comment: Projects may want to initiate passives or do other "BeginPlay" type of logic here.
    void OnAvatarSet( const FGameplayAbilityActorInfo * actor_info, const FGameplayAbilitySpec & spec ) override;

    // Same as calling K2_EndAbility. Meant for use with batching system to end the ability externally.
    void ExternalEndAbility();

    void SetCurrentMontageForMesh( USkeletalMeshComponent * mesh, UAnimMontage * current_montage );

    /** Returns the currently playing montage for this ability, if any */
    UFUNCTION( BlueprintPure, Category = Animation )
    UAnimMontage * GetCurrentMontageForMesh( USkeletalMeshComponent * mesh ) const;
    
    virtual int32 GetInputID() const;

protected:
    void EndAbility( const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo * actor_info, const FGameplayAbilityActivationInfo activation_info, bool replicate_end_ability, bool was_cancelled ) override;
    void OnGiveAbility( const FGameplayAbilityActorInfo * actor_info, const FGameplayAbilitySpec & spec ) override;
    void OnRemoveAbility( const FGameplayAbilityActorInfo * actor_info, const FGameplayAbilitySpec & spec ) override;

    UFUNCTION( BlueprintCallable )
    void AutoEndTaskWhenAbilityEnds( UAbilityTask * task );

    UFUNCTION( BlueprintImplementableEvent )
    void ReceiveOnGiveAbility( const FGameplayAbilityActorInfo & actor_info, const FGameplayAbilitySpec & spec );

    UFUNCTION( BlueprintImplementableEvent )
    void ReceiveOnRemoveAbility( const FGameplayAbilityActorInfo & actor_info, const FGameplayAbilitySpec & spec );

    // Tells an ability to activate immediately when its granted. Used for passive abilities and abilities forced on others.
    UPROPERTY( BlueprintReadOnly, EditDefaultsOnly, Category = "Ability" )
    uint8 bActivateAbilityOnGranted : 1;

    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Ability Activation" )
    EGASExtAbilityActivationGroup ActivationGroup;
private:
    bool FindAbilityMeshMontage( FGASExtAbilityMeshMontage & ability_mesh_montage, USkeletalMeshComponent * mesh ) const;

    /** Immediately jumps the active montage to a section */
    UFUNCTION( BlueprintCallable, Category = "Ability|Animation" )
    void MontageJumpToSectionForMesh( USkeletalMeshComponent * mesh, FName section_name );

    /** Sets pending section on active montage */
    UFUNCTION( BlueprintCallable, Category = "Ability|Animation" )
    void MontageSetNextSectionNameForMesh( USkeletalMeshComponent * mesh, FName from_section_name, FName to_section_name );

    /**
    * Stops the current animation montage.
    *
    * @param mesh
    * @param override_blend_out_time If >= 0, will override the BlendOutTime parameter on the AnimMontage instance
    */
    UFUNCTION( BlueprintCallable, Category = "Ability|Animation", Meta = ( AdvancedDisplay = "OverrideBlendOutTime" ) )
    void MontageStopForMesh( USkeletalMeshComponent * mesh, float override_blend_out_time = -1.0f );

    /**
    * Stops all currently animating montages
    *
    * @param override_blend_out_time If >= 0, will override the BlendOutTime parameter on the AnimMontage instance
    */
    UFUNCTION( BlueprintCallable, Category = "Ability|Animation", Meta = ( AdvancedDisplay = "OverrideBlendOutTime" ) )
    void MontageStopForAllMeshes( float override_blend_out_time = -1.0f );

    UFUNCTION( BlueprintCallable, Category = "GameplayEffects", meta = ( AutoCreateRefTerm = "event_data" ) )
    FGASExtGameplayEffectContainerSpec MakeEffectContainerSpecFromEffectContainer( const FGASExtGameplayEffectContainer & effect_container, const FGameplayEventData & event_data ) const;

    UFUNCTION( BlueprintPure, meta = ( DisplayName = "GetInstancingPolicy" ) )
    TEnumAsByte< EGameplayAbilityInstancingPolicy::Type > K2_GetInstancingPolicy() const;

    /** Active montages being played by this ability */
    UPROPERTY()
    TArray< FGASExtAbilityMeshMontage > CurrentAbilityMeshMontages;

    TArray< UAbilityTask * > TasksToEndWhenAbilityEnds;
};

FORCEINLINE EGASExtAbilityActivationGroup UGASExtGameplayAbility::GetActivationGroup() const
{
    return ActivationGroup;
}

FORCEINLINE TEnumAsByte< EGameplayAbilityInstancingPolicy::Type > UGASExtGameplayAbility::K2_GetInstancingPolicy() const
{
    return InstancingPolicy;
}