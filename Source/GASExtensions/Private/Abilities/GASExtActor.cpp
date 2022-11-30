#include "GASExtensions/Public/Abilities/GASExtActor.h"

#include "Abilities/GASExtAbilitySet.h"
#include "Components/GASExtAbilitySystemComponent.h"

AGASExtActor::AGASExtActor()
{
    AbilitySystemComponent = CreateDefaultSubobject< UGASExtAbilitySystemComponent >( TEXT( "AbilitySystemComponent" ) );
    AbilitySystemComponent->SetIsReplicated( true );

    // Mixed mode means we only are replicating the GEs to our-self, not the GEs to simulated proxies. If another SWEnemyBase receives a GE,
    // we won't be told about it by the Server. Attributes, GameplayTags, and GameplayCues will still replicate to us.
    AbilitySystemComponent->SetReplicationMode( EGameplayEffectReplicationMode::Mixed );
}

UAbilitySystemComponent * AGASExtActor::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

void AGASExtActor::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    check( AbilitySystemComponent != nullptr );
    AbilitySystemComponent->InitAbilityActorInfo( this, this );
}

void AGASExtActor::BeginPlay()
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
}
