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
    static UGASExtAT_WaitAbilityEnd * WaitAbilityEnd( UGameplayAbility * owning_ability,
        const FGameplayAbilitySpecHandle & ability_spec_handle,
        UAbilitySystemComponent * optional_asc = nullptr,
        bool trigger_once = true );

    void Activate() override;
    void OnDestroy( bool in_owner_finished ) override;

private:
    UFUNCTION()
    void OnAbilityEnded( const FAbilityEndedData & ability_ended_data );

    UPROPERTY( BlueprintAssignable )
    FGASExtOnAbilityEndedDelegate OnAbilityEndedDelegate;

    UPROPERTY()
    UAbilitySystemComponent * OptionalASC;

    bool bTriggerOnce;

    FGameplayAbilitySpecHandle AbilitySpecHandle;
    FDelegateHandle DelegateHandle;
};
