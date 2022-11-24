#include "Tasks/GASExtAT_WaitDelayWithProgress.h"

UGASExtAT_WaitDelayWithProgress * UGASExtAT_WaitDelayWithProgress::WaitDelayWithProgress( UGameplayAbility * owning_ability )
{
    auto * my_obj = NewAbilityTask< UGASExtAT_WaitDelayWithProgress >( owning_ability );
    return my_obj;
}

UGASExtAT_WaitDelayWithProgress * UGASExtAT_WaitDelayWithProgress::WaitDelayWithProgress( UGameplayAbility * owning_ability, float time, int progress_step, float optional_time_skip )
{
    auto * my_obj = NewAbilityTask< UGASExtAT_WaitDelayWithProgress >( owning_ability );
    my_obj->Time = time;
    my_obj->ProgressionPercentage = progress_step;
    my_obj->OptionalTimeSkip = optional_time_skip;
    return my_obj;
}

void UGASExtAT_WaitDelayWithProgress::Activate()
{
    Super::Activate();

    if ( OptionalTimeSkip >= Time )
    {
        return;
    }

    UWorld* World = GetWorld();
    TimeStarted = World->GetTimeSeconds();

    // Use a dummy timer handle as we don't need to store it for later but we don't need to look for something to clear
    FTimerHandle TimerHandle;
    World->GetTimerManager().SetTimer(TimerHandle, this, &UAbilityTask_WaitDelay::OnTimeFinish, Time, false);
}
