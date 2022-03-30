﻿#pragma once

#include <Abilities/Tasks/AbilityTask.h>

#include "GASExtAT_MonitorTriggerManager.generated.h"

class UGBFTriggerManagerComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FGASExtOnMonitorTriggerManagerDelegate, AActor *, activator, int, actor_count );

UCLASS()
class GASEXTENSIONS_API UGASExtAT_MonitorTriggerManager : public UAbilityTask
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable, Category = "Ability|Tasks", meta = ( HidePin = "owning_ability", DefaultToSelf = "owning_ability", BlueprintInternalUseOnly = "TRUE" ) )
    static UGASExtAT_MonitorTriggerManager * MonitorTriggerManager( UGameplayAbility * owning_ability, UGBFTriggerManagerComponent * trigger_manager_component );

    void Activate() override;
    void OnDestroy( bool bInOwnerFinished ) override;

protected:
    UPROPERTY( BlueprintAssignable )
    FGASExtOnMonitorTriggerManagerDelegate OnTriggerActivatedDelegate;

    UPROPERTY( BlueprintAssignable )
    FGASExtOnMonitorTriggerManagerDelegate OnActorInsideTriggerCountChangedDelegate;

private:
    UFUNCTION()
    void OnTriggerActivated( AActor * activator );

    UFUNCTION()
    void OnActorInsideTriggerCountChanged( int actor_count );

    UPROPERTY()
    UGBFTriggerManagerComponent * TriggerManagerComponent;
};
