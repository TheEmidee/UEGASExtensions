#include "Targeting/GASExtTargetDataFilter.h"

#include "AbilitySystemBlueprintLibrary.h"

FGameplayAbilityTargetDataHandle UGASExtTargetDataFilter::FilterTargetData( FGameplayAbilityTargetDataHandle target_data_handle ) const
{
    return UAbilitySystemBlueprintLibrary::FilterTargetData( target_data_handle, MakeFilterHandle() );
}

FGameplayTargetDataFilterHandle UGASExtTargetDataFilter::MakeFilterHandle() const
{
    return FGameplayTargetDataFilterHandle();
}

UGASExtTargetDataFilter_IsActorOfClass::UGASExtTargetDataFilter_IsActorOfClass()
{
    Filter.RequiredActorClass = RequiredActorClass;
    Filter.SelfFilter = ETargetDataFilterSelf::TDFS_Any;
}

FGameplayTargetDataFilterHandle UGASExtTargetDataFilter_IsActorOfClass::MakeFilterHandle() const
{
    FGameplayTargetDataFilterHandle handle;

    const auto new_filter = MakeShared< FGameplayTargetDataFilter >( Filter );
    new_filter->InitializeFilterContext( nullptr );
    handle.Filter = new_filter;

    return handle;
}
