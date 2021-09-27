#include "BehaviorTree/GASExtBTDecorator_CheckGameplayTagsOnActor.h"

#include <AbilitySystemBlueprintLibrary.h>
#include <BehaviorTree/Blackboard/BlackboardKeyType_Object.h>
#include <BehaviorTree/BlackboardComponent.h>

UGASExtBTDecorator_CheckGameplayTagsOnActor::UGASExtBTDecorator_CheckGameplayTagsOnActor( const FObjectInitializer & object_initializer ) :
    Super( object_initializer )
{
    NodeName = "GASExt Gameplay Tag Condition";

    bNotifyTick = false;

    bAllowAbortLowerPri = false;
    bAllowAbortNone = false;
    bAllowAbortChildNodes = true;
    FlowAbortMode = EBTFlowAbortMode::Self;

    bNotifyBecomeRelevant = true;
    bNotifyCeaseRelevant = true;
}

bool UGASExtBTDecorator_CheckGameplayTagsOnActor::CalculateRawConditionValue( UBehaviorTreeComponent & owner_comp, uint8 * node_memory ) const
{
    const auto * blackboard_comp = owner_comp.GetBlackboardComponent();
    if ( blackboard_comp == nullptr )
    {
        return false;
    }

    auto * asc = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(
        Cast< AActor >( blackboard_comp->GetValue< UBlackboardKeyType_Object >( ActorToCheck.GetSelectedKeyID() ) ) );

    if ( asc == nullptr )
    {
        return false;
    }

    switch ( TagsToMatch )
    {
        case EGameplayContainerMatchType::All:
            return asc->HasAllMatchingGameplayTags( GameplayTags );

        case EGameplayContainerMatchType::Any:
            return asc->HasAnyMatchingGameplayTags( GameplayTags );

        default:
        {
            UE_LOG( LogBehaviorTree, Warning, TEXT( "Invalid value for TagsToMatch (EGameplayContainerMatchType) %d.  Should only be Any or All." ), static_cast< int32 >( TagsToMatch ) );
            return false;
        }
    }
}

void UGASExtBTDecorator_CheckGameplayTagsOnActor::TickNode( UBehaviorTreeComponent & owner_comp, uint8 * node_memory, float delta_seconds )
{
    Super::TickNode( owner_comp, node_memory, delta_seconds );

    if ( !CalculateRawConditionValue( owner_comp, node_memory ) )
    {
        owner_comp.RequestExecution( this );
    }
}

void UGASExtBTDecorator_CheckGameplayTagsOnActor::OnBecomeRelevant( UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory )
{
    Super::OnBecomeRelevant( OwnerComp, NodeMemory );

    if ( const auto * blackboard_comp = OwnerComp.GetBlackboardComponent() )
    {
        if ( auto * asc = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(
                 Cast< AActor >( blackboard_comp->GetValue< UBlackboardKeyType_Object >( ActorToCheck.GetSelectedKeyID() ) ) ) )
        {
            OwnerBTComp = &OwnerComp;
            Handle = asc->RegisterGameplayTagEvent( GameplayTag, Type ).AddUObject( this, &UGASExtBTDecorator_CheckGameplayTagsOnActor::GameplayTagChanged );
        }
    }
}

void UGASExtBTDecorator_CheckGameplayTagsOnActor::OnCeaseRelevant( UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory )
{
    Super::OnCeaseRelevant( OwnerComp, NodeMemory );

    if ( const auto * blackboard_comp = OwnerComp.GetBlackboardComponent() )
    {
        if ( auto * asc = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(
                 Cast< AActor >( blackboard_comp->GetValue< UBlackboardKeyType_Object >( ActorToCheck.GetSelectedKeyID() ) ) ) )
        {
            asc->UnregisterGameplayTagEvent( Handle, GameplayTag, Type );
            OwnerBTComp = nullptr;
        }
    }
}

void UGASExtBTDecorator_CheckGameplayTagsOnActor::GameplayTagChanged( const FGameplayTag tag, int32 tag_event_type ) const
{
    if ( OwnerBTComp.IsValid() )
    {
        OwnerBTComp->RequestExecution( this );
    }
}