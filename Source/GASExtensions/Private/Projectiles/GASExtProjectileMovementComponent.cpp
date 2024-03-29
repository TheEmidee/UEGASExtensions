#include "Projectiles/GASExtProjectileMovementComponent.h"

#include <Components/PrimitiveComponent.h>
#include <Net/UnrealNetwork.h>

UGASExtProjectileMovementComponent::UGASExtProjectileMovementComponent() :
    UProjectileMovementComponent()
{
    HomingType = EGASExtProjectileHomingType::NoHoming;
    bUseTargetLocation = false;
    bInitRotationFollowVelocity = true;
}

void UGASExtProjectileMovementComponent::InitializeComponent()
{
    UMovementComponent::InitializeComponent();

    if ( Velocity.SizeSquared() > 0.f )
    {
        // InitialSpeed > 0 overrides initial velocity magnitude.
        if ( InitialSpeed > 0.f )
        {
            Velocity = Velocity.GetSafeNormal() * InitialSpeed;
        }

        if ( bInitialVelocityInLocalSpace )
        {
            SetVelocityInLocalSpace( Velocity );
        }

        // Copied this code here to add the boolean bInitRotationFollowVelocity
        // Without it the actor's rotation would always be reset at the start of the game if its velocity is 0.
        // This makes for unwanted re-orientating of certain actors.
        //
        if ( bRotationFollowsVelocity && bInitRotationFollowVelocity )
        {
            if ( UpdatedComponent )
            {
                FRotator DesiredRotation = Velocity.Rotation();
                if ( bRotationRemainsVertical )
                {
                    DesiredRotation.Pitch = 0.0f;
                    DesiredRotation.Yaw = FRotator::NormalizeAxis( DesiredRotation.Yaw );
                    DesiredRotation.Roll = 0.0f;
                }

                UpdatedComponent->SetWorldRotation( DesiredRotation );
            }
        }

        UpdateComponentVelocity();

        if ( UpdatedPrimitive && UpdatedPrimitive->IsSimulatingPhysics() )
        {
            UpdatedPrimitive->SetPhysicsLinearVelocity( Velocity );
        }
    }

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
    const auto target_location = bUseTargetLocation
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
    return bIsHomingProjectile && ( bUseTargetLocation || HomingTargetComponent.IsValid() );
}
