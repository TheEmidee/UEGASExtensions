#pragma once

#include <AbilitySystemGlobals.h>

#include "GASExtAbilitySystemGlobals.generated.h"

UCLASS()
class GASEXTENSIONS_API UGASExtAbilitySystemGlobals : public UAbilitySystemGlobals
{
    GENERATED_BODY()

    FGameplayEffectContext * AllocGameplayEffectContext() const override;
};

