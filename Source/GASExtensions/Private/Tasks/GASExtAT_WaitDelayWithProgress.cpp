#include "Tasks/GASExtAT_WaitDelayWithProgress.h"

#include <TimerManager.h>

UGASExtAT_WaitDelayWithProgress * UGASExtAT_WaitDelayWithProgress::WaitDelayWithProgress( UGameplayAbility * owning_ability, float time, int progress_percentage, float optional_time_skip /*= 0.0f*/ )
{
    auto * my_obj = NewAbilityTask< UGASExtAT_WaitDelayWithProgress >( owning_ability );
    my_obj->Time = time;
    my_obj->ProgressionPercentage = progress_percentage;
    my_obj->OptionalTimeSkip = optional_time_skip;
    return my_obj;
}

void UGASExtAT_WaitDelayWithProgress::Activate()
{
    Super::Activate();

    if ( OptionalTimeSkip >= Time || Time <= 0.0f )
    {
        if ( ShouldBroadcastAbilityTaskDelegates() )
        {
            OnDelayFinishedDelegate.Broadcast( 100.0f );
        }
        return;
    }

    ProgressRate = Time * ( ProgressionPercentage / 100.0f );
    RemainingTime = Time - OptionalTimeSkip;

    UpdateTimer();
}

void UGASExtAT_WaitDelayWithProgress::OnProgressUpdate()
{
    if ( ShouldBroadcastAbilityTaskDelegates() )
    {
        if ( FMath::IsNearlyZero( RemainingTime, 0.0001f ) )
        {
            OnDelayFinishedDelegate.Broadcast( 100.0f );
            EndTask();
            return;
        }

        OnProgressUpdateDelegate.Broadcast( ( 1.0f - ( RemainingTime / Time ) ) * 100.0f );
        UpdateTimer();
    }
}

void UGASExtAT_WaitDelayWithProgress::UpdateTimer()
{
    const auto rate = ProgressRate < RemainingTime ? ProgressRate : RemainingTime;
    RemainingTime -= rate;

    const auto * world = GetWorld();
    world->GetTimerManager().SetTimer( TimerHandle, this, &UGASExtAT_WaitDelayWithProgress::OnProgressUpdate, rate, false );
}
