#include "Tasks/GASExtAT_WaitPrimitiveComponentHit.h"

UGASExtAT_WaitPrimitiveComponentHit * UGASExtAT_WaitPrimitiveComponentHit::WaitPrimitiveComponentHit( UGameplayAbility * owning_ability, UPrimitiveComponent * component )
{
    auto * my_obj = NewAbilityTask< UGASExtAT_WaitPrimitiveComponentHit >( owning_ability );
    my_obj->PrimitiveComponent = component;
    return my_obj;
}

void UGASExtAT_WaitPrimitiveComponentHit::Activate()
{
    Super::Activate();

    SetWaitingOnAvatar();

    if ( auto * primitive_component = GetPrimitiveComponent() )
    {
        primitive_component->OnComponentHit.AddDynamic( this, &ThisClass::OnComponentHit );
    }
}

void UGASExtAT_WaitPrimitiveComponentHit::OnDestroy( bool ability_ended )
{
    if ( auto * primitive_component = GetPrimitiveComponent() )
    {
        primitive_component->OnComponentHit.RemoveDynamic( this, &ThisClass::OnComponentHit );
    }

    Super::OnDestroy( ability_ended );
}

UPrimitiveComponent * UGASExtAT_WaitPrimitiveComponentHit::GetPrimitiveComponent()
{
    if ( PrimitiveComponent == nullptr )
    {
        if ( const auto * avatar_actor = GetAvatarActor() )
        {
            PrimitiveComponent = Cast< UPrimitiveComponent >( avatar_actor->GetRootComponent() );
            if ( PrimitiveComponent == nullptr )
            {
                PrimitiveComponent = avatar_actor->FindComponentByClass< UPrimitiveComponent >();
            }
        }
    }

    return PrimitiveComponent;
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void UGASExtAT_WaitPrimitiveComponentHit::OnComponentHit( UPrimitiveComponent * hit_component, AActor * other_actor, UPrimitiveComponent * /*other_comp*/, FVector /*normal_impulse*/, const FHitResult & hit_result )
{
    if ( other_actor != nullptr )
    {
        auto * target_data = new FGameplayAbilityTargetData_SingleTargetHit( hit_result );

        FGameplayAbilityTargetDataHandle handle;
        handle.Data.Add( TSharedPtr< FGameplayAbilityTargetData >( target_data ) );

        if ( ShouldBroadcastAbilityTaskDelegates() )
        {
            OnComponentHitDelegate.Broadcast( handle );
        }

        // We are done. Kill us so we don't keep getting broadcast messages
        EndTask();
    }
}
