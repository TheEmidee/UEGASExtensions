#pragma once

#include <Abilities/Tasks/AbilityTask.h>
#include <CoreMinimal.h>

#include "GASExtAT_WaitDelayWithProgress.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE( FGASExtOnDelayFinished );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FGASExtOnProgressUpdate, float, progress_percentage );

UCLASS()
class GASEXTENSIONS_API UGASExtAT_WaitDelayWithProgress final : public UAbilityTask
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable, Category = "Ability|Tasks", meta = ( HidePin = "owning_ability", DefaultToSelf = "owning_ability", BlueprintInternalUseOnly = "TRUE" ) )
    static UGASExtAT_WaitDelayWithProgress * WaitDelayWithProgress( UGameplayAbility * owning_ability, float time, int progress_step, float optional_time_skip );

    void Activate() override;

private:
    float Time;
    int ProgressionPercentage;
    float OptionalTimeSkip;
};