#include "GASExtAbilitySystemGlobals.h"

FGameplayEffectContext * UGASExtAbilitySystemGlobals::AllocGameplayEffectContext() const
{
    return new FGASExtGameplayEffectContext();
}
