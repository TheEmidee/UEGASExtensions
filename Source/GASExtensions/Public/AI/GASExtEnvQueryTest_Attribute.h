#pragma once

#include <CoreMinimal.h>
#include <EnvironmentQuery/EnvQueryTest.h>

#include "GASExtEnvQueryTest_Attribute.generated.h"

UCLASS()
class GASEXTENSIONS_API UGASExtEnvQueryTest_Attribute final : public UEnvQueryTest
{
    GENERATED_BODY()

protected:
    void RunTest( FEnvQueryInstance & query_instance ) const override;
    FText GetDescriptionDetails() const override;

    UPROPERTY( EditAnywhere, Category = "Attribute" )
    FGameplayAttribute Attribute;

    // This controls how to treat actors that do not implement IGameplayTagAssetInterface.
    // When set to True, actors that do not implement the interface will be ignored, meaning
    // they will not be scored and will not be considered when filtering.
    // When set to False, actors that do not implement the interface will be included in
    // filter and score operations with a zero score.
    UPROPERTY( EditAnywhere, Category = "Attribute" )
    bool bRejectIncompatibleItems;
};
