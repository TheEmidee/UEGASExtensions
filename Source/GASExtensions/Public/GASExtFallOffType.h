#pragma once

UCLASS( Blueprintable, HideDropdown )
class GASEXTENSIONS_API UGASExtFallOffType : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintPure )
    virtual float GetFallOffMultiplier( const float distance );

    UPROPERTY( EditAnywhere )
    float Radius;
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
