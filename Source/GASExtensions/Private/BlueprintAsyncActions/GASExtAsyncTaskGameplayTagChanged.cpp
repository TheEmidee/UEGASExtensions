#include "BlueprintAsyncActions/GASExtAsyncTaskGameplayTagChanged.h"

#include <AbilitySystemComponent.h>

UGASExtAsyncTaskGameplayTagChanged * UGASExtAsyncTaskGameplayTagChanged::ListenForGameplayTagChange( UAbilitySystemComponent * ability_system_component, const FGameplayTag gameplay_tag, const EGameplayTagEventType::Type tag_event_type )
{
    auto wait_for_gameplay_tag_changed_task = NewObject< UGASExtAsyncTaskGameplayTagChanged >();
    wait_for_gameplay_tag_changed_task->ASC = ability_system_component;
    wait_for_gameplay_tag_changed_task->GameplayTagToListenFor = gameplay_tag;
    wait_for_gameplay_tag_changed_task->TagEventType = tag_event_type;

    if ( !IsValid( ability_system_component ) || !gameplay_tag.IsValid() )
    {
        wait_for_gameplay_tag_changed_task->RemoveFromRoot();
        return nullptr;
    }

    wait_for_gameplay_tag_changed_task->ListenForGameplayTagChangeDelegateHandle =
        ability_system_component->RegisterGameplayTagEvent( gameplay_tag, tag_event_type ).AddUObject( wait_for_gameplay_tag_changed_task, &GameplayTagChanged );

    return wait_for_gameplay_tag_changed_task;
}

UGASExtAsyncTaskGameplayTagChanged * UGASExtAsyncTaskGameplayTagChanged::ListenForAnyGameplayTagChange( UAbilitySystemComponent * ability_system_component )
{
    auto wait_for_any_gameplay_tag_changed_task = NewObject< UGASExtAsyncTaskGameplayTagChanged >();
    wait_for_any_gameplay_tag_changed_task->ASC = ability_system_component;

    if ( !IsValid( ability_system_component ) )
    {
        wait_for_any_gameplay_tag_changed_task->RemoveFromRoot();
        return nullptr;
    }

    wait_for_any_gameplay_tag_changed_task->ListenForGameplayTagChangeDelegateHandle =
        ability_system_component->RegisterGenericGameplayTagEvent().AddUObject( wait_for_any_gameplay_tag_changed_task, &GameplayTagChanged );

    return wait_for_any_gameplay_tag_changed_task;
}

void UGASExtAsyncTaskGameplayTagChanged::EndTask()
{
    if ( IsValid( ASC ) )
    {
        if ( GameplayTagToListenFor.IsValid() )
        {
            ASC->UnregisterGameplayTagEvent( ListenForGameplayTagChangeDelegateHandle, GameplayTagToListenFor, TagEventType );
        }
        else
        {
            ASC->RegisterGenericGameplayTagEvent().Remove( ListenForGameplayTagChangeDelegateHandle );
        }
    }

    SetReadyToDestroy();
    MarkPendingKill();
}

void UGASExtAsyncTaskGameplayTagChanged::GameplayTagChanged( const FGameplayTag tag, int32 tag_event_type ) const
{
    OnGameplayTagChangedDelegate.Broadcast( tag, static_cast< EGameplayTagEventType::Type >( tag_event_type ), ASC->HasMatchingGameplayTag( tag ) );
}
