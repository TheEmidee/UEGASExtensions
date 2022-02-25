#include "GASExtensions/Public/Abilities/GASExtActor.h"

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
