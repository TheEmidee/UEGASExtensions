#pragma once

#include <AbilitySystemInterface.h>
#include <CoreMinimal.h>
#include <GameFramework/Actor.h>

#include "GASExtActorWithAbilities.generated.h"

class UGASExtAbilitySet;
class UGASExtAbilityTagRelationshipMapping;
class UGASExtAbilitySystemComponent;

UCLASS()
class GASEXTENSIONS_API AGASExtActorWithAbilities : public AActor, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    AGASExtActorWithAbilities();

    UAbilitySystemComponent * GetAbilitySystemComponent() const override;
    void PostInitializeComponents() override;
    void BeginPlay() override;

private:
    UPROPERTY( VisibleAnywhere, BlueprintReadOnly, meta = ( AllowPrivateAccess = "true" ) )
    UGASExtAbilitySystemComponent * AbilitySystemComponent;

    UPROPERTY( EditDefaultsOnly )
    UGASExtAbilityTagRelationshipMapping * TagRelationshipMapping;

    UPROPERTY( EditDefaultsOnly )
    TArray< const UGASExtAbilitySet * > AbilitySets;

    UPROPERTY( EditDefaultsOnly )
    EGameplayEffectReplicationMode GameplayEffectReplicationMode;

    UPROPERTY( EditDefaultsOnly )
    TArray< TSubclassOf< UGameplayEffect > > AdditionalDefaultEffects;
};
