#include "AI/GASExtEnvQueryTest_GameplayAttribute.h"

#include <AbilitySystemBlueprintLibrary.h>

void UGASExtEnvQueryTest_GameplayAttribute::RunTest( FEnvQueryInstance & query_instance ) const
{
    const UObject * query_owner = query_instance.Owner.Get();
    if ( query_owner == nullptr )
    {
        return;
    }

    if ( !UAbilitySystemBlueprintLibrary::IsValid( Attribute ) )
    {
        return;
    }

    BoolValue.BindData( query_owner, query_instance.QueryID );
    const auto wants_valid = BoolValue.GetValue();

    FloatValueMin.BindData( query_owner, query_instance.QueryID );
    const auto min_threshold_value = FloatValueMin.GetValue();

    FloatValueMax.BindData( query_owner, query_instance.QueryID );
    const auto max_threshold_value = FloatValueMax.GetValue();

    const EEnvItemStatus::Type incompatible_status = bRejectIncompatibleItems ? EEnvItemStatus::Failed : EEnvItemStatus::Passed;

    for ( FEnvQueryInstance::ItemIterator item_iterator( this, query_instance ); item_iterator; ++item_iterator )
    {
        const AActor * item_actor = GetItemActor( query_instance, item_iterator.GetIndex() );

        bool attribute_was_found = false;
        const auto attribute_value = UAbilitySystemBlueprintLibrary::GetFloatAttribute( item_actor, Attribute, attribute_was_found );

        if ( !attribute_was_found )
        {
            item_iterator.ForceItemState( incompatible_status );
            continue;
        }

        item_iterator.SetScore( TestPurpose, FilterType, attribute_value, min_threshold_value, max_threshold_value );
    }
}

FText UGASExtEnvQueryTest_GameplayAttribute::GetDescriptionDetails() const
{
    return FText::FromString( Attribute.GetName() );
}

FText UGASExtEnvQueryTest_GameplayAttribute::GetDescriptionTitle() const
{
    return FText::FromString( FString::Printf( TEXT( "%s: %s" ),
        *Super::GetDescriptionTitle().ToString(),
        *Attribute.GetName() ) );
}
