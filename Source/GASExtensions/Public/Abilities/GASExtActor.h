#pragma once

#include <AbilitySystemInterface.h>
#include <CoreMinimal.h>

#include "GASExtActor.generated.h"

UCLASS()
class GASEXTENSIONS_API AGASExtActor : public AActor, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    UAbilitySystemComponent * GetAbilitySystemComponent() const override;

private:
    UPROPERTY( VisibleAnywhere, BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    UGASExtAbilitySystemComponent * AbilitySystemComponent;
};
