#include "Tasks/GASExtAT_WaitTargetData.h"

#include <AbilitySystemComponent.h>

FGASExtWaitTargetDataReplicationOptions::FGASExtWaitTargetDataReplicationOptions()
{
    ShouldProduceTargetDataOnServer = false;
    CreateKeyIfNotValidForMorePredicting = true;
}

void UGASExtAT_WaitTargetData::Activate()
{
    if ( IsPendingKill() )
    {
        return;
    }

    check( IsValid( Ability ) );

    // If not locally controlled (server for remote client), see if TargetData was already sent
    // else register callback for when it does get here.
    if ( !Ability->GetCurrentActorInfo()->IsLocallyControlled() )
    {
        // Register with the TargetData callbacks if we are expecting client to send them
        if ( !ReplicationOptions.ShouldProduceTargetDataOnServer )
        {
            const auto spec_handle = GetAbilitySpecHandle();
            const auto activation_prediction_key = GetActivationPredictionKey();

            //Since multi-fire is supported, we still need to hook up the callbacks
            AbilitySystemComponent->AbilityTargetDataSetDelegate( spec_handle, activation_prediction_key ).AddUObject( this, &UGASExtAT_WaitTargetData::OnTargetDataReplicatedCallback );
            AbilitySystemComponent->CallReplicatedTargetDataDelegatesIfSet( spec_handle, activation_prediction_key );
            SetWaitingOnRemotePlayerData();
        }
    }

    if ( ShouldProduceTargetData() )
    {
        const auto target_data_handle = ProduceTargetData();
        SendTargetData( target_data_handle );
    }
}

void UGASExtAT_WaitTargetData::OnDestroy( const bool ability_ended )
{
    Super::OnDestroy( ability_ended );

    if ( IsValid( AbilitySystemComponent ) )
    {
        const auto spec_handle = GetAbilitySpecHandle();
        const auto activation_prediction_key = GetActivationPredictionKey();
        AbilitySystemComponent->AbilityTargetDataSetDelegate( spec_handle, activation_prediction_key ).RemoveAll( this );
    }
}

// ReSharper disable once CppMemberFunctionMayBeStatic
bool UGASExtAT_WaitTargetData::OnReplicatedTargetDataReceived( const FGameplayAbilityTargetDataHandle & data ) const
{
    return true;
}

void UGASExtAT_WaitTargetData::OnTargetDataReplicatedCallback( const FGameplayAbilityTargetDataHandle & data, FGameplayTag activation_tag )
{
    check( AbilitySystemComponent );

    AbilitySystemComponent->ConsumeClientReplicatedTargetData( GetAbilitySpecHandle(), GetActivationPredictionKey() );

    if ( !OnReplicatedTargetDataReceived( data ) )
    {
        if ( ShouldBroadcastAbilityTaskDelegates() )
        {
            Cancelled.Broadcast( data );
        }
    }
    else if ( ShouldBroadcastAbilityTaskDelegates() )
    {
        ValidData.Broadcast( data );
    }

    EndTask();
}

void UGASExtAT_WaitTargetData::SendTargetData( const FGameplayAbilityTargetDataHandle & data )
{
    check( IsValid( AbilitySystemComponent ) );
    if ( Ability == nullptr )
    {
        return;
    }

    FScopedPredictionWindow scoped_prediction(
        AbilitySystemComponent,
        ShouldReplicateDataToServer() && ( ReplicationOptions.CreateKeyIfNotValidForMorePredicting && !AbilitySystemComponent->ScopedPredictionKey.IsValidForMorePrediction() ) );

    if ( IsPredictingClient() )
    {
        if ( !ReplicationOptions.ShouldProduceTargetDataOnServer )
        {
            static const FGameplayTag ApplicationTag; // Fixme: where would this be useful?

            AbilitySystemComponent->CallServerSetReplicatedTargetData(
                GetAbilitySpecHandle(),
                GetActivationPredictionKey(),
                data,
                ApplicationTag,
                AbilitySystemComponent->ScopedPredictionKey );
        }
    }

    if ( ShouldBroadcastAbilityTaskDelegates() )
    {
        ValidData.Broadcast( data );
    }

    EndTask();
}

bool UGASExtAT_WaitTargetData::ShouldReplicateDataToServer() const
{
    if ( Ability == nullptr )
    {
        return false;
    }

    const auto actor_info = Ability->GetCurrentActorInfo();
    return !actor_info->IsNetAuthority();
}

bool UGASExtAT_WaitTargetData::ShouldProduceTargetData() const
{
    // return true if we are locally owned, or (we are the server and this is a gameplay target ability that can produce target data server side)
    return Ability->GetCurrentActorInfo()->IsLocallyControlled() || ReplicationOptions.ShouldProduceTargetDataOnServer;
}