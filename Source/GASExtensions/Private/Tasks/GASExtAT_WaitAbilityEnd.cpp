#include "Tasks/GASExtAT_WaitAbilityEnd.h"

UGASExtAT_WaitAbilityEnd * UGASExtAT_WaitAbilityEnd::WaitAbilityEnd( UGameplayAbility * owning_ability, const FGameplayAbilitySpecHandle & ability_spec_handle, UAbilitySystemComponent * optional_asc /*= nullptr*/ )
{
    auto * my_obj = NewAbilityTask< UGASExtAT_WaitAbilityEnd >( owning_ability );
    my_obj->OptionalASC = optional_asc;
    my_obj->AbilitySpecHandle = ability_spec_handle;
    return my_obj;
}

void UGASExtAT_WaitAbilityEnd::Activate()
{
    Super::Activate();

    SetWaitingOnAvatar();

    if ( !IsValid( OptionalASC ) )
    {
        OptionalASC = AbilitySystemComponent;
    }

    DelegateHandle = OptionalASC->OnAbilityEnded.AddUObject( this, &UGASExtAT_WaitAbilityEnd::OnAbilityEnded );
}

void UGASExtAT_WaitAbilityEnd::OnAbilityEnded( const FAbilityEndedData & ability_ended_data )
{
    if ( ability_ended_data.AbilitySpecHandle == AbilitySpecHandle )
    {
        OnAbilityEndedDelegate.Broadcast( ability_ended_data.bWasCancelled );
        OptionalASC->OnAbilityEnded.Remove( DelegateHandle );
    }
}