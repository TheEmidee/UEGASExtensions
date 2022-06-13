#include "Abilities/GASExtAbilityTagRelationshipMapping.h"

void UGASExtAbilityTagRelationshipMapping::GetAbilityTagsToBlockAndCancel( const FGameplayTagContainer & ability_tags, FGameplayTagContainer * tags_to_block, FGameplayTagContainer * tags_to_cancel ) const
{
    for ( const auto & tags : AbilityTagRelationships )
    {
        if ( !ability_tags.HasTag( tags.AbilityTag ) )
        {
            continue;
        }

        if ( tags_to_block )
        {
            tags_to_block->AppendTags( tags.AbilityTagsToBlock );
        }
        if ( tags_to_cancel )
        {
            tags_to_cancel->AppendTags( tags.AbilityTagsToCancel );
        }
    }
}

void UGASExtAbilityTagRelationshipMapping::GetRequiredAndBlockedActivationTags( const FGameplayTagContainer & ability_tags, FGameplayTagContainer * activation_required, FGameplayTagContainer * activation_blocked ) const
{
    for ( const auto & tags : AbilityTagRelationships )
    {
        if ( !ability_tags.HasTag( tags.AbilityTag ) )
        {
            continue;
        }

        if ( activation_required )
        {
            activation_required->AppendTags( tags.ActivationRequiredTags );
        }
        if ( activation_blocked )
        {
            activation_blocked->AppendTags( tags.ActivationBlockedTags );
        }
    }
}

bool UGASExtAbilityTagRelationshipMapping::IsAbilityCancelledByTag( const FGameplayTagContainer & ability_tags, const FGameplayTag & action_tag ) const
{
    for ( const auto & tags : AbilityTagRelationships )
    {
        if ( tags.AbilityTag == action_tag && tags.AbilityTagsToCancel.HasAny( ability_tags ) )
        {
            return true;
        }
    }

    return false;
}
