#pragma once

#include <Abilities/Tasks/AbilityTask.h>
#include <CoreMinimal.h>

#include "GASExtAT_WaitPrimitiveComponentHit.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnWaitPrimitiveComponentHitDelegate, const FGameplayAbilityTargetDataHandle &, TargetData );

UCLASS()
class GASEXTENSIONS_API UGASExtAT_WaitPrimitiveComponentHit final : public UAbilityTask
{
    GENERATED_BODY()

    /*
     * If PrimitiveComponent is null, the task will try to find one on the Avatar Actor of the gameplay ability
     */
    UFUNCTION( BlueprintCallable, Category = "Ability|Tasks", meta = ( HidePin = "owning_ability", DefaultToSelf = "owning_ability", BlueprintInternalUseOnly = "TRUE" ) )
    static UGASExtAT_WaitPrimitiveComponentHit * WaitPrimitiveComponentHit( UGameplayAbility * owning_ability, UPrimitiveComponent * component );

    void Activate() override;

protected:
    UPROPERTY( BlueprintAssignable )
    FOnWaitPrimitiveComponentHitDelegate OnComponentHitDelegate;

private:
    void OnDestroy( bool ability_ended ) override;
    UPrimitiveComponent * GetPrimitiveComponent();

    UFUNCTION()
    void OnComponentHit( UPrimitiveComponent * hit_component, AActor * other_actor, UPrimitiveComponent * other_comp, FVector normal_impulse, const FHitResult & hit_result );

    UPROPERTY()
    UPrimitiveComponent * PrimitiveComponent;
};
