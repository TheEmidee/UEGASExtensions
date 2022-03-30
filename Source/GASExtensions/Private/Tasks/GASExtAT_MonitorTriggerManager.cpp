#include "Tasks/GASExtAT_MonitorTriggerManager.h"

#include "Gameplay/Components/GBFTriggerManagerComponent.h"

UGASExtAT_MonitorTriggerManager * UGASExtAT_MonitorTriggerManager::MonitorTriggerManager( UGameplayAbility * owning_ability, UGBFTriggerManagerComponent * trigger_manager_component )
{
    auto * my_obj = NewAbilityTask< UGASExtAT_MonitorTriggerManager >( owning_ability );
    my_obj->TriggerManagerComponent = trigger_manager_component;
    return my_obj;
}

void UGASExtAT_MonitorTriggerManager::Activate()
{
    Super::Activate();

    SetWaitingOnAvatar();

    if ( TriggerManagerComponent != nullptr )
    {
        TriggerManagerComponent->OnTriggerBoxActivated().AddDynamic( this, &UGASExtAT_MonitorTriggerManager::OnTriggerActivated );
        TriggerManagerComponent->OnActorInsideTriggerCountChanged().AddDynamic( this, &UGASExtAT_MonitorTriggerManager::OnActorInsideTriggerCountChanged );
    }
}

void UGASExtAT_MonitorTriggerManager::OnDestroy( bool bInOwnerFinished )
{
    if ( TriggerManagerComponent != nullptr )
    {
        TriggerManagerComponent->OnTriggerBoxActivated().RemoveDynamic( this, &UGASExtAT_MonitorTriggerManager::OnTriggerActivated );
        TriggerManagerComponent->OnActorInsideTriggerCountChanged().RemoveDynamic( this, &UGASExtAT_MonitorTriggerManager::OnActorInsideTriggerCountChanged );
    }

    Super::OnDestroy( bInOwnerFinished );
}

void UGASExtAT_MonitorTriggerManager::OnTriggerActivated( AActor * activator )
{
    if ( ShouldBroadcastAbilityTaskDelegates() )
    {
        OnTriggerActivatedDelegate.Broadcast( activator, TriggerManagerComponent->GetActorsInTrigger().Num() );
    }
}

void UGASExtAT_MonitorTriggerManager::OnActorInsideTriggerCountChanged( int actor_count )
{
    if ( ShouldBroadcastAbilityTaskDelegates() )
    {
        const auto actors_in_trigger = TriggerManagerComponent->GetActorsInTrigger();
        const auto last_actor = actors_in_trigger.Num() > 0 ? actors_in_trigger.Last() : nullptr;

        OnActorInsideTriggerCountChangedDelegate.Broadcast( last_actor, actor_count );
    }
}
