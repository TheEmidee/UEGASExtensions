#pragma once

#include "GameplayEffectTypes.h"

#include <CoreMinimal.h>
#include <GameplayTagContainer.h>
#include <Kismet/BlueprintAsyncActionBase.h>

#include "GASExtAsyncTaskGameplayTagChanged.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams( FSWOnGameplayTagChangedDelegate, FGameplayTag, gameplay_tag, EGameplayTagEventType::Type, tag_event_type, bool, it_is_present );

class UAbilitySystemComponent;

UCLASS()
class GASEXTENSIONS_API UGASExtAsyncTaskGameplayTagChanged : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable, meta = ( BlueprintInternalUseOnly = true ) )
    static UGASExtAsyncTaskGameplayTagChanged * ListenForGameplayTagChange( UAbilitySystemComponent * ability_system_component, FGameplayTag gameplay_tag, EGameplayTagEventType::Type tag_event_type );

    UFUNCTION( BlueprintCallable, meta = ( BlueprintInternalUseOnly = true ) )
    static UGASExtAsyncTaskGameplayTagChanged * ListenForAnyGameplayTagChange( UAbilitySystemComponent * ability_system_component );

    UFUNCTION( BlueprintCallable )
    void EndTask();

    UPROPERTY( BlueprintAssignable )
    FSWOnGameplayTagChangedDelegate OnGameplayTagChangedDelegate;

protected:
    UFUNCTION()
    void GameplayTagChanged( FGameplayTag tag, int32 tag_event_type ) const;

    UPROPERTY()
    UAbilitySystemComponent * ASC;

    FGameplayTag GameplayTagToListenFor;
    EGameplayTagEventType::Type TagEventType;

private:
    FDelegateHandle ListenForGameplayTagChangeDelegateHandle;
};
