#pragma once

#include <AbilitySystemComponent.h>
#include <AbilitySystemInterface.h>
#include <CoreMinimal.h>
#include <GameFramework/Actor.h>

#include "GASExtActorWithAbilities.generated.h"

class UGameplayEffect;
class UGASExtAbilitySet;
class UGASExtAbilityTagRelationshipMapping;
class UGASExtAbilitySystemComponent;

UCLASS()
class GASEXTENSIONS_API AGASExtActorWithAbilities : public AActor, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    explicit AGASExtActorWithAbilities( const FObjectInitializer & object_initializer );

    UAbilitySystemComponent * GetAbilitySystemComponent() const override;
    void PostInitializeComponents() override;
    void BeginPlay() override;

protected:
    UPROPERTY( VisibleAnywhere, BlueprintReadOnly, meta = ( AllowPrivateAccess = "true" ) )
    TObjectPtr< UGASExtAbilitySystemComponent > AbilitySystemComponent;

private:
    UPROPERTY( EditDefaultsOnly )
    TObjectPtr< UGASExtAbilityTagRelationshipMapping > TagRelationshipMapping;

    UPROPERTY( EditDefaultsOnly )
    TArray< TObjectPtr< const UGASExtAbilitySet > > AbilitySets;

    UPROPERTY( EditDefaultsOnly )
    EGameplayEffectReplicationMode GameplayEffectReplicationMode;

    UPROPERTY( EditDefaultsOnly )
    TArray< TSubclassOf< UGameplayEffect > > AdditionalDefaultEffects;

    UPROPERTY( EditDefaultsOnly )
    TArray< TSubclassOf< UAttributeSet > > AttributeSetClasses;

    UPROPERTY( Transient )
    TArray< TObjectPtr< UAttributeSet > > AttributeSets;
};
