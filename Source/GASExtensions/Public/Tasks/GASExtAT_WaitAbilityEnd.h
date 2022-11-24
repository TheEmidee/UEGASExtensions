#pragma once

#include <Abilities/Tasks/AbilityTask.h>
#include <CoreMinimal.h>

#include "GASExtAT_WaitAbilityEnd.generated.h"

class UPrimitiveComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FGASExtOnAbilityEndedDelegate, bool, was_cancelled );

UCLASS()
class GASEXTENSIONS_API UGASExtAT_WaitAbilityEnd final : public UAbilityTask
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable, Category = "Ability|Tasks", meta = ( HidePin = "owning_ability", DefaultToSelf = "owning_ability", BlueprintInternalUseOnly = "TRUE" ) )
    static UGASExtAT_WaitAbilityEnd * WaitAbilityEnd( UGameplayAbility * owning_ability, UAbilitySystemComponent * asc, const FGameplayAbilitySpecHandle & ability_spec_handle );

    void Activate() override;

protected:
    UPROPERTY( BlueprintAssignable )
    FGASExtOnAbilityEndedDelegate OnAbilityEndedDelegate;

private:
    void OnDestroy( bool ability_ended ) override;

    UFUNCTION()
    void OnAbilityEnded( const FAbilityEndedData & ability_ended_data );

    UPROPERTY()
    UAbilitySystemComponent * ASC;

    FGameplayAbilitySpecHandle AbilitySpecHandle;
    FDelegateHandle DelegateHandle;
};
