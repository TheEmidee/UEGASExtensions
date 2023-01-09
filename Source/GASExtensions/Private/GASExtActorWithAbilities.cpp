#include "GASExtActorWithAbilities.h"

#include "Abilities/GASExtAbilitySet.h"
#include "Components/GASExtAbilitySystemComponent.h"

AGASExtActorWithAbilities::AGASExtActorWithAbilities()
{
    PrimaryActorTick.bCanEverTick = false;

    AbilitySystemComponent = CreateDefaultSubobject< UGASExtAbilitySystemComponent >( TEXT( "AbilitySystemComponent" ) );
    AbilitySystemComponent->SetIsReplicated( true );
    AbilitySystemComponent->SetReplicationMode( GameplayEffectReplicationMode );
}

UAbilitySystemComponent * AGASExtActorWithAbilities::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

void AGASExtActorWithAbilities::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    check( AbilitySystemComponent != nullptr );
    AbilitySystemComponent->InitAbilityActorInfo( this, this );
}

void AGASExtActorWithAbilities::BeginPlay()
{
    Super::BeginPlay();

    if ( !HasAuthority() )
    {
        return;
    }

    for ( const auto * ability_set : AbilitySets )
    {
        ability_set->GiveToAbilitySystem( AbilitySystemComponent, nullptr );
    }

    AbilitySystemComponent->SetTagRelationshipMapping( TagRelationshipMapping );

    for ( const auto & effect : AdditionalDefaultEffects )
    {
        const auto spec_handle = AbilitySystemComponent->MakeOutgoingSpec( effect, 0, FGameplayEffectContextHandle() );
        AbilitySystemComponent->ApplyGameplayEffectSpecToSelf( *spec_handle.Data.Get() );
    }

    for ( const auto attribute_set_class : AttributeSetClasses )
    {
        auto * attribute_set = NewObject< UAttributeSet >( this, attribute_set_class, NAME_None, RF_Transient );
        AbilitySystemComponent->GetSpawnedAttributes_Mutable().Add( attribute_set );
        AttributeSets.Add( attribute_set );
    }
}