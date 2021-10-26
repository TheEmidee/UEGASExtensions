#pragma once

#include "GASExtFallOffType.generated.h"

class UCurveFloat;

UCLASS( NotBlueprintable, HideDropdown, EditInlineNew )
class GASEXTENSIONS_API UGASExtFallOffType : public UObject
{
    GENERATED_BODY()

public:
    UGASExtFallOffType();

    UFUNCTION( BlueprintPure )
    virtual float GetFallOffMultiplier( const float distance );

    UFUNCTION( BlueprintPure )
    float GetRadius() const;

#if WITH_EDITOR
    void PostEditChangeProperty( FPropertyChangedEvent & property_changed_event ) override;
#endif

private:
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
