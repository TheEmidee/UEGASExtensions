#include "BehaviorTree/GASExtBTDecorator_CheckGameplayTagsOnActor.h"

#include <AbilitySystemBlueprintLibrary.h>
#include <BehaviorTree/Blackboard/BlackboardKeyType_Object.h>
#include <BehaviorTree/BlackboardComponent.h>

UGASExtBTDecorator_CheckGameplayTagsOnActor::UGASExtBTDecorator_CheckGameplayTagsOnActor( const FObjectInitializer & object_initializer ) :
    Super( object_initializer )
{
    NodeName = "GASExt Gameplay Tag Condition";

    bNotifyTick = false;

    bAllowAbortLowerPri = true;
    bAllowAbortNone = false;
    bAllowAbortChildNodes = true;
    FlowAbortMode = EBTFlowAbortMode::Self;

    bNotifyBecomeRelevant = true;
    bNotifyCeaseRelevant = true;

    // Accept only actors
    ActorToCheck.AddObjectFilter( this, GET_MEMBER_NAME_CHECKED( UGASExtBTDecorator_CheckGameplayTagsOnActor, ActorToCheck ), AActor::StaticClass() );

    // Default to using Self Actor
    ActorToCheck.SelectedKeyName = FBlackboard::KeySelf;
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

void UGASExtBTDecorator_CheckGameplayTagsOnActor::OnBecomeRelevant( UBehaviorTreeComponent & owner_comp, uint8 * node_memory )
{
    Super::OnBecomeRelevant( owner_comp, node_memory );

    if ( const auto * blackboard_comp = owner_comp.GetBlackboardComponent() )
    {
        if ( auto * asc = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(
                 Cast< AActor >( blackboard_comp->GetValue< UBlackboardKeyType_Object >( ActorToCheck.GetSelectedKeyID() ) ) ) )
        {
            OwnerComp = &owner_comp;
            GameplayTagChangedHandle = asc->RegisterGenericGameplayTagEvent().AddUObject( this, &UGASExtBTDecorator_CheckGameplayTagsOnActor::GameplayTagChanged );
        }
    }
}

void UGASExtBTDecorator_CheckGameplayTagsOnActor::OnCeaseRelevant( UBehaviorTreeComponent & owner_comp, uint8 * node_memory )
{
    Super::OnCeaseRelevant( owner_comp, node_memory );

    if ( const auto * blackboard_comp = owner_comp.GetBlackboardComponent() )
    {
        if ( auto * asc = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(
                 Cast< AActor >( blackboard_comp->GetValue< UBlackboardKeyType_Object >( ActorToCheck.GetSelectedKeyID() ) ) ) )
        {
            asc->RegisterGenericGameplayTagEvent().Remove( GameplayTagChangedHandle );
            OwnerComp.Reset();
        }
    }
}

void UGASExtBTDecorator_CheckGameplayTagsOnActor::GameplayTagChanged( const FGameplayTag tag, int32 tag_event_type ) const
{
    if ( OwnerComp.IsValid() && GameplayTags.HasTag( tag ) )
    {
        OwnerComp->RequestExecution( this );
    }
}

void UGASExtBTDecorator_CheckGameplayTagsOnActor::InitializeFromAsset( UBehaviorTree & asset )
{
    Super::InitializeFromAsset( asset );

    auto * blackboard_asset = GetBlackboardAsset();
    if ( ensure( blackboard_asset ) )
    {
        ActorToCheck.ResolveSelectedKey( *blackboard_asset );
    }
}
