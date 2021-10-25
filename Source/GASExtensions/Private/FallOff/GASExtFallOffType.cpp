#include "FallOff/GASExtFallOffType.h"

#include "DVEDataValidator.h"
#include "Log/CoreExtLog.h"

#include <Curves/CurveFloat.h>

float UGASExtFallOffType::GetFallOffMultiplier( const float /*distance*/ )
{
    return 0.0f;
}

void UGASExtFallOffType::PostEditChangeProperty( FPropertyChangedEvent & property_changed_event )
{
    Super::PostEditChangeProperty( property_changed_event );

    if ( Radius.GetValue() <= 0.0f )
    {
        Radius.Value = 1.0f;
        UE_SLOG( LogTemp, Warning, TEXT( "Radius cannot be smaller than or equal to 0!" ) );
    }
}

float UGASExtFallOffType_Linear::GetFallOffMultiplier( const float distance )
{
    if ( ensureAlwaysMsgf( Radius.GetValue() > 0.0f, TEXT( "Radius cannot be smaller than or equal to 0!" ) ) )
    {
        return 1.0f - distance / Radius.GetValue();
    }

    return 0.0f;
}

float UGASExtFallOffType_Inversed::GetFallOffMultiplier( const float distance )
{
    if ( ensureAlwaysMsgf( Radius.GetValue() > 0.0f, TEXT( "Radius cannot be smaller than or equal to 0!" ) ) )
    {
        return distance / Radius.GetValue();
    }

    return 0.0f;
}

float UGASExtFallOffType_Squared::GetFallOffMultiplier( const float distance )
{
    if ( ensureAlwaysMsgf( Radius.GetValue() > 0.0f, TEXT( "Radius cannot be smaller than or equal to 0!" ) ) )
    {
        return 1.0f - FMath::Square( distance / Radius.GetValue() );
    }

    return 0.0f;
}

float UGASExtFallOffType_Logarithmic::GetFallOffMultiplier( const float distance )
{
    if ( ensureAlwaysMsgf( Radius.GetValue() > 0.0f, TEXT( "Radius cannot be smaller than or equal to 0!" ) ) )
    {
        return -FMath::LogX( 10, distance / Radius.GetValue() );
    }

    return 0.0f;
}

float UGASExtFallOffType_Curve::GetFallOffMultiplier( const float distance )
{
    if ( ensureAlwaysMsgf( Radius.GetValue() > 0.0f, TEXT( "Radius cannot be smaller than or equal to 0!" ) ) )
    {
        return Curve->GetFloatValue( distance / Radius.GetValue() );
    }

    return 0.0f;
}
