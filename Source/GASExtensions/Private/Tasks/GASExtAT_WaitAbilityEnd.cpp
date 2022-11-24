#include "Tasks/GASExtAT_WaitAbilityEnd.h"

UGASExtAT_WaitAbilityEnd * UGASExtAT_WaitAbilityEnd::WaitAbilityEnd( UGameplayAbility * owning_ability, UAbilitySystemComponent * asc, const FGameplayAbilitySpecHandle & ability_spec_handle )
{
    auto * my_obj = NewAbilityTask< UGASExtAT_WaitAbilityEnd >( owning_ability );
    my_obj->ASC = asc;
    my_obj->AbilitySpecHandle = ability_spec_handle;
    return my_obj;
}

void UGASExtAT_WaitAbilityEnd::Activate()
{
    Super::Activate();

    SetWaitingOnAvatar();

    DelegateHandle = ASC->OnAbilityEnded.AddUObject( this, &UGASExtAT_WaitAbilityEnd::OnAbilityEnded );
}

void UGASExtAT_WaitAbilityEnd::OnDestroy( const bool ability_ended )
{
    ASC->OnAbilityEnded.Remove( DelegateHandle );

    Super::OnDestroy( ability_ended );
}

void UGASExtAT_WaitAbilityEnd::OnAbilityEnded( const FAbilityEndedData & ability_ended_data )
{
    if ( ability_ended_data.AbilitySpecHandle == AbilitySpecHandle )
    {
        OnAbilityEndedDelegate.Broadcast( ability_ended_data.bWasCancelled );
        BeginDestroy();
    }
}
