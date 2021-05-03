#include "Projectiles/GASExtProjectileMovementComponent.h"

#include <Net/UnrealNetwork.h>

UGASExtProjectileMovementComponent::UGASExtProjectileMovementComponent() :
    UProjectileMovementComponent()
{
    HomingType = EGASExtProjectileHomingType::NoHoming;
    UseTargetLocation = false;
}

void UGASExtProjectileMovementComponent::InitializeComponent()
{
    Super::InitializeComponent();
    SetHomingType( HomingType );
}

FVector UGASExtProjectileMovementComponent::ComputeAcceleration( const FVector & velocity, const float delta_time ) const
{
    auto acceleration( FVector::ZeroVector );

    acceleration.Z += GetGravityZ();

    acceleration += PendingForceThisUpdate;

    if ( IsHomingValid() )
    {
        acceleration += ComputeHomingAcceleration( velocity, delta_time );
    }

    return acceleration;
}

FVector UGASExtProjectileMovementComponent::ComputeHomingAcceleration( const FVector & /*in_velocity*/, float /*delta_time*/ ) const
{
    const auto target_location = UseTargetLocation
                                     ? TargetLocation
                                     : HomingTargetComponent->GetComponentLocation();
    const auto direction = ( target_location - UpdatedComponent->GetComponentLocation() ).GetSafeNormal();
    const auto homing_acceleration = direction * HomingAccelerationMagnitude;
    return homing_acceleration;
}

bool UGASExtProjectileMovementComponent::ShouldUseSubStepping() const
{
    return bForceSubStepping ||
           ( GetGravityZ() != 0.f ) ||
           ( IsHomingValid() );
}

// ReSharper disable once CppInconsistentNaming
void UGASExtProjectileMovementComponent::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const
{
    Super::GetLifetimeReplicatedProps( OutLifetimeProps );
    DOREPLIFETIME( UGASExtProjectileMovementComponent, TargetLocation );
}

bool UGASExtProjectileMovementComponent::IsHomingValid() const
{
    return bIsHomingProjectile && ( UseTargetLocation || HomingTargetComponent.IsValid() );
}
