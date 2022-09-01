#pragma once

#include <AbilitySystemInterface.h>
#include <CoreMinimal.h>
#include <GameFramework/Actor.h>

#include "GASExtActor.generated.h"

class UGASExtAbilitySystemComponent;
class UGASExtAbilityTagRelationshipMapping;
class UGASExtAbilitySet;

UCLASS()
class GASEXTENSIONS_API AGASExtActor : public AActor, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    AGASExtActor();

    UAbilitySystemComponent * GetAbilitySystemComponent() const override;
    void PostInitializeComponents() override;
    void BeginPlay() override;

private:
    UPROPERTY( VisibleAnywhere, BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    UGASExtAbilitySystemComponent * AbilitySystemComponent;

    UPROPERTY( EditDefaultsOnly )
    UGASExtAbilityTagRelationshipMapping * TagRelationshipMapping;

    UPROPERTY( EditDefaultsOnly )
    TArray< const UGASExtAbilitySet * > AbilitySets;
};
