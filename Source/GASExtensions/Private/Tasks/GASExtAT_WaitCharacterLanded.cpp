#include "Tasks/GASExtAT_WaitCharacterLanded.h"

UGASExtAT_WaitCharacterLanded * UGASExtAT_WaitCharacterLanded::WaitCharacterLanded( UGameplayAbility * owning_ability, ACharacter * character )
{
    auto * my_obj = NewAbilityTask< UGASExtAT_WaitCharacterLanded >( owning_ability );
    my_obj->Character = character;
    return my_obj;
}

void UGASExtAT_WaitCharacterLanded::Activate()
{
    Super::Activate();

    SetWaitingOnAvatar();

    if ( Character != nullptr )
    {
        Character->LandedDelegate.AddDynamic( this, &UGASExtAT_WaitCharacterLanded::OnCharacterLanded );
    }
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UGASExtAT_WaitCharacterLanded::OnCharacterLanded( const FHitResult & hit_result )
{
    OnCharacterLandedDelegate.Broadcast( hit_result );
}

void UGASExtAT_WaitCharacterLanded::OnDestroy( const bool ability_ended )
{
    if ( Character != nullptr )
    {
        Character->LandedDelegate.RemoveDynamic( this, &UGASExtAT_WaitCharacterLanded::OnCharacterLanded );
    }

    Super::OnDestroy( ability_ended );
}
