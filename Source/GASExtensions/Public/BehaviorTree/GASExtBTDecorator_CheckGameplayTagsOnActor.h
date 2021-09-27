#pragma once

#include <BehaviorTree/Decorators/BTDecorator_CheckGameplayTagsOnActor.h>
#include <CoreMinimal.h>
#include <GameplayEffectTypes.h>

#include "GASExtBTDecorator_CheckGameplayTagsOnActor.generated.h"

UCLASS()
class GASEXTENSIONS_API UGASExtBTDecorator_CheckGameplayTagsOnActor : public UBTDecorator_CheckGameplayTagsOnActor
{
    GENERATED_BODY()

public:
    UGASExtBTDecorator_CheckGameplayTagsOnActor( const FObjectInitializer & object_initializer );
    bool CalculateRawConditionValue( UBehaviorTreeComponent & owner_comp, uint8 * node_memory ) const override;

    void TickNode( UBehaviorTreeComponent & owner_comp, uint8 * node_memory, float delta_seconds ) override;

protected:
    void OnBecomeRelevant( UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory ) override;
    void OnCeaseRelevant( UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory ) override;

    UFUNCTION()
    void GameplayTagChanged( const FGameplayTag tag, int32 tag_event_type ) const;

    UPROPERTY( EditAnywhere, Category = GameplayTagCheck )
    FGameplayTag GameplayTag;

    UPROPERTY( EditAnywhere, Category = GameplayTagCheck )
    TEnumAsByte< EGameplayTagEventType::Type > Type;

    FDelegateHandle Handle;
    TWeakObjectPtr< UBehaviorTreeComponent > OwnerBTComp;
};
