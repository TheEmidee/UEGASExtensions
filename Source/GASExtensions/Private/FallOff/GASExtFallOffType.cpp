#include "FallOff/GASExtFallOffType.h"

#include "DVEDataValidator.h"

#include <Curves/CurveFloat.h>

float UGASExtFallOffType::GetFallOffMultiplier( const float /*distance*/ )
{
    return 0.0f;
}

EDataValidationResult UGASExtFallOffType::IsDataValid( TArray< FText > & validation_errors )
{
    return FDVEDataValidator( validation_errors )
        .CustomValidation< FScalableFloat >( Radius, []( TArray< FText > & errors, FScalableFloat radius ) {
            if ( radius.GetValue() <= 0.0f )
            {
                errors.Add( FText::FromString( TEXT( "Radius cannot be 0 or smaller" ) ) );
            }
        } )
        .Result();
}

float UGASExtFallOffType_Linear::GetFallOffMultiplier( const float distance )
{
    return 1.0f - distance / Radius.GetValue();
}

float UGASExtFallOffType_Inversed::GetFallOffMultiplier( const float distance )
{
    return distance / Radius.GetValue();
}

float UGASExtFallOffType_Squared::GetFallOffMultiplier( const float distance )
{
    return 1.0f - FMath::Square( distance / Radius.GetValue() );
}

float UGASExtFallOffType_Logarithmic::GetFallOffMultiplier( const float distance )
{
    return -FMath::LogX( 10, distance / Radius.GetValue() );
}

float UGASExtFallOffType_Curve::GetFallOffMultiplier( const float distance )
{
    return Curve->GetFloatValue( distance / Radius.GetValue() );
}
