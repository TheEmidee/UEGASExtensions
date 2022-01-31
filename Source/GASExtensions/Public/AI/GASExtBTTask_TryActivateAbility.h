#pragma once

#include <BehaviorTree/Tasks/BTTask_BlackboardBase.h>
#include <CoreMinimal.h>

#include "GASExtBTTask_TryActivateAbility.generated.h"

struct FGASExtTryActivateAbilityBTTaskMemory
{
    FGASExtTryActivateAbilityBTTaskMemory();

    FOnGameplayAbilityEnded::FDelegate OnGameplayAbilityEndedDelegate;
    FGameplayAbilitySpecHandle AbilitySpecHandle;
    TWeakObjectPtr< UAbilitySystemComponent > ASC;
    bool bAbilityHasEnded;
    bool bObserverCanFinishTask;
};

UCLASS( Abstract )
class GASEXTENSIONS_API UGASExtBTTask_TryActivateAbility : public UBTTask_BlackboardBase
{
    GENERATED_BODY()

public:
    explicit UGASExtBTTask_TryActivateAbility( const FObjectInitializer & object_initializer );

    EBTNodeResult::Type ExecuteTask( UBehaviorTreeComponent & owner_comp, uint8 * node_memory ) override;
    uint16 GetInstanceMemorySize() const override;
    FString GetStaticDescription() const override;

protected:
    void OnTaskFinished( UBehaviorTreeComponent & owner_comp, uint8 * node_memory, EBTNodeResult::Type task_result ) override;
    virtual FString GetDetailedStaticDescription() const PURE_VIRTUAL( UGASExtBTTask_TryActivateAbility::GetDetailedStaticDescription, return ""; );
    EBTNodeResult::Type AbortTask( UBehaviorTreeComponent & owner_comp, uint8 * node_memory ) override;
    virtual EBTNodeResult::Type TryActivateAbility( UBehaviorTreeComponent & owner_comp, UAbilitySystemComponent & asc, FGASExtTryActivateAbilityBTTaskMemory * memory ) PURE_VIRTUAL( UGASExtBTTask_TryActivateAbility::TryActivateAbility, return EBTNodeResult::Aborted; );
    EBTNodeResult::Type TryActivateAbilityHandle( UBehaviorTreeComponent & owner_comp, UAbilitySystemComponent & asc, FGameplayAbilitySpecHandle ability_to_activate, FGASExtTryActivateAbilityBTTaskMemory * memory );

private:
    void OnGameplayAbilityEnded( UGameplayAbility * ability, FGASExtTryActivateAbilityBTTaskMemory * memory, UBehaviorTreeComponent * owner_comp );
};

UCLASS()
class GASEXTENSIONS_API UGASExtBTTask_TryActivateAbilityByClass final : public UGASExtBTTask_TryActivateAbility
{
    GENERATED_BODY()

public:
    explicit UGASExtBTTask_TryActivateAbilityByClass( const FObjectInitializer & object_initializer );

protected:
    FString GetDetailedStaticDescription() const override;
    EBTNodeResult::Type TryActivateAbility( UBehaviorTreeComponent & owner_comp, UAbilitySystemComponent & asc, FGASExtTryActivateAbilityBTTaskMemory * memory ) override;

private:
    UPROPERTY( Category = Node, EditAnywhere )
    TSubclassOf< UGameplayAbility > AbilityClass;
};

/*
 * Try to activate an ability by tag.
 * Note that only the first matching ability will be activated !
 */
UCLASS()
class GASEXTENSIONS_API UGASExtBTTask_TryActivateAbilityByTag final : public UGASExtBTTask_TryActivateAbility
{
    GENERATED_BODY()

public:
    explicit UGASExtBTTask_TryActivateAbilityByTag( const FObjectInitializer & object_initializer );

protected:
    FString GetDetailedStaticDescription() const override;
    EBTNodeResult::Type TryActivateAbility( UBehaviorTreeComponent & owner_comp, UAbilitySystemComponent & asc, FGASExtTryActivateAbilityBTTaskMemory * memory ) override;

private:
    UPROPERTY( Category = Node, EditAnywhere )
    FGameplayTagContainer TagContainer;
};