#pragma once

#include <BehaviorTree/Decorators/BTDecorator_CheckGameplayTagsOnActor.h>
#include <CoreMinimal.h>
#include <GameplayEffectTypes.h>

#include "GASExtBTDecorator_CheckGameplayTagsOnActor.generated.h"

UCLASS()
class GASEXTENSIONS_API UGASExtBTDecorator_CheckGameplayTagsOnActor : public UBTDecorator
{
    GENERATED_BODY()

public:
    UGASExtBTDecorator_CheckGameplayTagsOnActor( const FObjectInitializer & object_initializer );
    bool CalculateRawConditionValue( UBehaviorTreeComponent & owner_comp, uint8 * node_memory ) const override;

protected:
    void OnBecomeRelevant( UBehaviorTreeComponent & owner_comp, uint8 * node_memory ) override;
    void OnCeaseRelevant( UBehaviorTreeComponent & owner_comp, uint8 * node_memory ) override;

    UFUNCTION()
    void GameplayTagChanged( const FGameplayTag tag, int32 tag_event_type ) const;

    void InitializeFromAsset( UBehaviorTree & asset ) override;

    UPROPERTY( EditAnywhere, Category = GameplayTagCheck, Meta = ( ToolTips = "Which Actor (from the blackboard) should be checked for these gameplay tags?" ) )
    struct FBlackboardKeySelector ActorToCheck;

    UPROPERTY( EditAnywhere, Category = GameplayTagCheck )
    EGameplayContainerMatchType TagsToMatch;

    UPROPERTY( EditAnywhere, Category = GameplayTagCheck )
    FGameplayTagContainer GameplayTags;

    UPROPERTY( EditAnywhere, Category = GameplayTagCheck )
    TEnumAsByte< EGameplayTagEventType::Type > TagEventType;

    FDelegateHandle GameplayTagChangedHandle;
    TWeakObjectPtr< UBehaviorTreeComponent > OwnerComp;
};
