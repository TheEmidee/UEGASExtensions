#include "GASExtAbilitySystemGlobals.h"

#include "GASExtAbilityTypesBase.h"

FGameplayEffectContext * UGASExtAbilitySystemGlobals::AllocGameplayEffectContext() const
{
    return new FGASExtGameplayEffectContext();
}
