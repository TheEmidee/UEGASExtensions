#pragma once

#include <CoreMinimal.h>
#include <UObject/NoExportTypes.h>

#include "GASExtTargetDataFilter.generated.h"

UCLASS( NotBlueprintable, EditInlineNew, HideDropdown, meta = ( ShowWorldContextPin ) )
class GASEXTENSIONS_API UGASExtTargetDataFilter : public UObject
{
    GENERATED_BODY()

public:
    FGameplayAbilityTargetDataHandle FilterTargetData( FGameplayAbilityTargetDataHandle target_data ) const;

protected:
    virtual FGameplayTargetDataFilterHandle MakeFilterHandle() const;
};

UCLASS()
class GASEXTENSIONS_API UGASExtTargetDataFilter_IsActorOfClass final : public UGASExtTargetDataFilter
{
    GENERATED_BODY()

public:
    UGASExtTargetDataFilter_IsActorOfClass();

protected:
    FGameplayTargetDataFilterHandle MakeFilterHandle() const override;

    FGameplayTargetDataFilter Filter;

    UPROPERTY( EditAnywhere )
    TSubclassOf< AActor > RequiredActorClass;
};