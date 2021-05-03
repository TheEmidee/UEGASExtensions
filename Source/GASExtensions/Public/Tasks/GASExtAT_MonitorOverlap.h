#pragma once

#include <Abilities/Tasks/AbilityTask.h>
#include <CoreMinimal.h>

#include "GASExtAT_MonitorOverlap.generated.h"

class UPrimitiveComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FOnMonitorOverlapDelegate, AActor *, OtherActor, UPrimitiveComponent *, OtherComponent );

UCLASS()
class GASEXTENSIONS_API UGASExtAT_MonitorOverlap final : public UAbilityTask
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable, Category = "Ability|Tasks", meta = ( HidePin = "owning_ability", DefaultToSelf = "owning_ability", BlueprintInternalUseOnly = "TRUE" ) )
    static UGASExtAT_MonitorOverlap * MonitorOverlap( UGameplayAbility * owning_ability, UPrimitiveComponent * component );

    void Activate() override;

protected:
    UPROPERTY( BlueprintAssignable )
    FOnMonitorOverlapDelegate OnComponentBeginOverlapDelegate;

    UPROPERTY( BlueprintAssignable )
    FOnMonitorOverlapDelegate OnComponentEndOverlapDelegate;

private:
    void OnDestroy( bool ability_ended ) override;

    UFUNCTION()
    void OnComponentBeginOverlap( UPrimitiveComponent * overlapped_component, AActor * other_actor, UPrimitiveComponent * other_component, int32 other_body_index, bool from_sweep, const FHitResult & hit_result );

    UFUNCTION()
    void OnComponentEndOverlap( UPrimitiveComponent * overlapped_component, AActor * other_actor, UPrimitiveComponent * other_component, int32 other_body_index );

    UPROPERTY()
    UPrimitiveComponent * PrimitiveComponent;
};