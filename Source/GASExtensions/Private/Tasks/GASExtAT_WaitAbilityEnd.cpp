#include "Tasks/GASExtAT_WaitAbilityEnd.h"

#include <AbilitySystemComponent.h>

UGASExtAT_WaitAbilityEnd * UGASExtAT_WaitAbilityEnd::WaitAbilityEnd( UGameplayAbility * owning_ability,
    const FGameplayAbilitySpecHandle & ability_spec_handle,
    UAbilitySystemComponent * optional_asc /*= nullptr*/,
    bool trigger_once /*= true*/ )
{
    auto * my_obj = NewAbilityTask< UGASExtAT_WaitAbilityEnd >( owning_ability );
    my_obj->OptionalASC = optional_asc;
    my_obj->AbilitySpecHandle = ability_spec_handle;
    my_obj->bTriggerOnce = trigger_once;
    return my_obj;
}

void UGASExtAT_WaitAbilityEnd::Activate()
{
    Super::Activate();

    SetWaitingOnAvatar();

    if ( OptionalASC != nullptr )
    {
        OptionalASC = AbilitySystemComponent;
    }

    DelegateHandle = OptionalASC->OnAbilityEnded.AddUObject( this, &UGASExtAT_WaitAbilityEnd::OnAbilityEnded );
}

void UGASExtAT_WaitAbilityEnd::OnDestroy( bool in_owner_finished )
{
    OptionalASC->OnAbilityEnded.Remove( DelegateHandle );

    Super::OnDestroy( in_owner_finished );
}

void UGASExtAT_WaitAbilityEnd::OnAbilityEnded( const FAbilityEndedData & ability_ended_data )
{
    if ( ShouldBroadcastAbilityTaskDelegates() )
    {
        if ( ability_ended_data.AbilitySpecHandle == AbilitySpecHandle )
        {
            OnAbilityEndedDelegate.Broadcast( ability_ended_data.bWasCancelled );

            if ( bTriggerOnce )
            {
                EndTask();
            }
        }
    }
}
