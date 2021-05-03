#include "Tasks/GASExtAT_MonitorOverlap.h"

UGASExtAT_MonitorOverlap * UGASExtAT_MonitorOverlap::MonitorOverlap( UGameplayAbility * owning_ability, UPrimitiveComponent * component )
{
    auto * my_obj = NewAbilityTask< UGASExtAT_MonitorOverlap >( owning_ability );
    my_obj->PrimitiveComponent = component;
    return my_obj;
}

void UGASExtAT_MonitorOverlap::Activate()
{
    Super::Activate();

    SetWaitingOnAvatar();

    if ( PrimitiveComponent != nullptr )
    {
        PrimitiveComponent->OnComponentBeginOverlap.AddDynamic( this, &UGASExtAT_MonitorOverlap::OnComponentBeginOverlap );
        PrimitiveComponent->OnComponentEndOverlap.AddDynamic( this, &UGASExtAT_MonitorOverlap::OnComponentEndOverlap );
    }
}

void UGASExtAT_MonitorOverlap::OnDestroy( const bool ability_ended )
{
    if ( PrimitiveComponent != nullptr )
    {
        PrimitiveComponent->OnComponentBeginOverlap.RemoveDynamic( this, &UGASExtAT_MonitorOverlap::OnComponentBeginOverlap );
        PrimitiveComponent->OnComponentEndOverlap.RemoveDynamic( this, &UGASExtAT_MonitorOverlap::OnComponentEndOverlap );
    }

    Super::OnDestroy( ability_ended );
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UGASExtAT_MonitorOverlap::OnComponentBeginOverlap( UPrimitiveComponent * /*overlapped_component*/, AActor * other_actor, UPrimitiveComponent * other_component, int32 /*other_body_index*/, bool /*from_sweep*/, const FHitResult & /*hit_result*/ )
{
    if ( ShouldBroadcastAbilityTaskDelegates() )
    {
        OnComponentBeginOverlapDelegate.Broadcast( other_actor, other_component );
    }
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UGASExtAT_MonitorOverlap::OnComponentEndOverlap( UPrimitiveComponent * /*overlapped_component*/, AActor * other_actor, UPrimitiveComponent * other_component, int32 /*other_body_index*/ )
{
    if ( ShouldBroadcastAbilityTaskDelegates() )
    {
        OnComponentEndOverlapDelegate.Broadcast( other_actor, other_component );
    }
}
