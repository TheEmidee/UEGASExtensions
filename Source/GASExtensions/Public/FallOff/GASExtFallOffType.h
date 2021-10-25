#pragma once

#include "GASExtFallOffType.generated.h"

class UCurveFloat;

UCLASS( NotBlueprintable, HideDropdown, EditInlineNew )
class GASEXTENSIONS_API UGASExtFallOffType : public UObject
{
    GENERATED_BODY()

public:
    UGASExtFallOffType()
    {
        Radius = 100.0f;
    }

    UFUNCTION( BlueprintPure )
    virtual float GetFallOffMultiplier( const float distance );

    EDataValidationResult IsDataValid( TArray< FText > & validation_errors ) override;

    UPROPERTY( EditAnywhere )
    FScalableFloat Radius;
};

UCLASS()
class GASEXTENSIONS_API UGASExtFallOffType_Linear : public UGASExtFallOffType
{
    GENERATED_BODY()

public:
    float GetFallOffMultiplier( const float distance ) override;
};

UCLASS()
class GASEXTENSIONS_API UGASExtFallOffType_Inversed : public UGASExtFallOffType
{
    GENERATED_BODY()

public:
    float GetFallOffMultiplier( const float distance ) override;
};

UCLASS()
class GASEXTENSIONS_API UGASExtFallOffType_Squared : public UGASExtFallOffType
{
    GENERATED_BODY()

public:
    float GetFallOffMultiplier( const float distance ) override;
};

UCLASS()
class GASEXTENSIONS_API UGASExtFallOffType_Logarithmic : public UGASExtFallOffType
{
    GENERATED_BODY()

public:
    float GetFallOffMultiplier( const float distance ) override;
};

UCLASS()
class GASEXTENSIONS_API UGASExtFallOffType_Curve : public UGASExtFallOffType
{
    GENERATED_BODY()

public:
    float GetFallOffMultiplier( const float distance ) override;

    UPROPERTY( EditAnywhere )
    UCurveFloat * Curve;
};
