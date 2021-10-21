#include "GASExtFallOffType.h"

#include "Curves/CurveFloat.h"

float UGASExtFallOffType::GetFallOffMultiplier( const float /*distance*/ )
{
    return 0.0f;
}

float UGASExtFallOffType_Linear::GetFallOffMultiplier( const float distance )
{
    return 1.0f - distance / Radius;
}

float UGASExtFallOffType_Inversed::GetFallOffMultiplier( const float distance )
{
    return distance / Radius;
}

float UGASExtFallOffType_Squared::GetFallOffMultiplier( const float distance )
{
    return 1.0f - FMath::Square( distance / Radius );
}

float UGASExtFallOffType_Logarithmic::GetFallOffMultiplier( const float distance )
{
    return -FMath::LogX( 10, distance / Radius );
}

float UGASExtFallOffType_Curve::GetFallOffMultiplier( const float distance )
{
    return Curve->GetFloatValue( distance / Radius );
}
