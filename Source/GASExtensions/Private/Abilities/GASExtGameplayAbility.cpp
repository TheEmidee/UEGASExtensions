#include "Abilities/GASExtGameplayAbility.h"

#include "GASExtAbilitySystemFunctionLibrary.h"

#include <Abilities/Tasks/AbilityTask.h>
#include <Components/GASExtAbilitySystemComponent.h>
#include <GameFramework/PlayerController.h>
#include <GameplayTask.h>

UGASExtGameplayAbility::UGASExtGameplayAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    ActivationGroup = EGASExtAbilityActivationGroup::Independent;
    bActivateAbilityOnGranted = false;
}

bool UGASExtGameplayAbility::K2_IsLocallyControlled() const
{
    return IsLocallyControlled();
}

UGASExtAbilitySystemComponent * UGASExtGameplayAbility::GetGASExtAbilitySystemComponent() const
{
    if ( auto * asc = GetAbilitySystemComponentFromActorInfo_Ensured() )
    {
        return Cast< UGASExtAbilitySystemComponent >( asc );
    }

    return nullptr;
}

UGASExtAbilitySystemComponent * UGASExtGameplayAbility::GetGASExtAbilitySystemComponentFromActorInfo() const
{
    return CurrentActorInfo
               ? Cast< UGASExtAbilitySystemComponent >( CurrentActorInfo->AbilitySystemComponent.Get() )
               : nullptr;
}

AController * UGASExtGameplayAbility::GetControllerFromActorInfo() const
{
    if ( CurrentActorInfo )
    {
        if ( auto * pc = CurrentActorInfo->PlayerController.Get() )
        {
            return pc;
        }

        // Look for a player controller or pawn in the owner chain.
        auto * test_actor = CurrentActorInfo->OwnerActor.Get();
        while ( test_actor )
        {
            if ( auto * controller = Cast< AController >( test_actor ) )
            {
                return controller;
            }

            if ( const auto * pawn = Cast< APawn >( test_actor ) )
            {
                return pawn->GetController();
            }

            test_actor = test_actor->GetOwner();
        }
    }

    return nullptr;
}

void UGASExtGameplayAbility::OnAvatarSet( const FGameplayAbilityActorInfo * actor_info, const FGameplayAbilitySpec & spec )
{
    Super::OnAvatarSet( actor_info, spec );

    if ( bActivateAbilityOnGranted )
    {
        actor_info->AbilitySystemComponent->TryActivateAbility( spec.Handle, false );
    }
}

void UGASExtGameplayAbility::ExternalEndAbility()
{
    check( CurrentActorInfo != nullptr );

    const auto replicate_end_ability = true;
    const auto was_cancelled = false;

    EndAbility( CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, replicate_end_ability, was_cancelled );
}

void UGASExtGameplayAbility::SetCurrentMontageForMesh( USkeletalMeshComponent * mesh, UAnimMontage * current_montage )
{
    ensure( IsInstantiated() );

    FGASExtAbilityMeshMontage ability_mesh_montage;
    if ( FindAbilityMeshMontage( ability_mesh_montage, mesh ) )
    {
        ability_mesh_montage.Montage = current_montage;
    }
    else
    {
        CurrentAbilityMeshMontages.Add( FGASExtAbilityMeshMontage( mesh, current_montage ) );
    }
}

UAnimMontage * UGASExtGameplayAbility::GetCurrentMontageForMesh( USkeletalMeshComponent * mesh ) const
{
    FGASExtAbilityMeshMontage ability_mesh_montage;
    if ( FindAbilityMeshMontage( ability_mesh_montage, mesh ) )
    {
        return ability_mesh_montage.Montage;
    }

    return nullptr;
}

int32 UGASExtGameplayAbility::GetInputID() const
{
    return INDEX_NONE;
}

void UGASExtGameplayAbility::EndAbility( const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo * actor_info, const FGameplayAbilityActivationInfo activation_info, const bool replicate_end_ability, const bool was_cancelled )
{
    Super::EndAbility( handle, actor_info, activation_info, replicate_end_ability, was_cancelled );

    for ( auto * task : TasksToEndWhenAbilityEnds )
    {
        if ( IsValid( task ) && task->GetState() != EGameplayTaskState::Finished )
        {
            task->EndTask();
        }
    }

    TasksToEndWhenAbilityEnds.Reset();
}

void UGASExtGameplayAbility::OnGiveAbility( const FGameplayAbilityActorInfo * actor_info, const FGameplayAbilitySpec & spec )
{
    Super::OnGiveAbility( actor_info, spec );

    ReceiveOnGiveAbility( *actor_info, spec );
}

void UGASExtGameplayAbility::OnRemoveAbility( const FGameplayAbilityActorInfo * actor_info, const FGameplayAbilitySpec & spec )
{
    Super::OnRemoveAbility( actor_info, spec );

    ReceiveOnRemoveAbility( *actor_info, spec );
}

void UGASExtGameplayAbility::AutoEndTaskWhenAbilityEnds( UAbilityTask * task )
{
    if ( task != nullptr )
    {
        TasksToEndWhenAbilityEnds.AddUnique( task );
    }
}

bool UGASExtGameplayAbility::FindAbilityMeshMontage( FGASExtAbilityMeshMontage & ability_mesh_montage, USkeletalMeshComponent * mesh ) const
{
    for ( const auto & mesh_montage : CurrentAbilityMeshMontages )
    {
        if ( mesh_montage.Mesh == mesh )
        {
            ability_mesh_montage = mesh_montage;
            return true;
        }
    }

    return false;
}

void UGASExtGameplayAbility::MontageJumpToSectionForMesh( USkeletalMeshComponent * mesh, const FName section_name )
{
    check( CurrentActorInfo != nullptr );

    if ( auto * ability_system_component = Cast< UGASExtAbilitySystemComponent >( GetAbilitySystemComponentFromActorInfo_Checked() ) )
    {
        if ( ability_system_component->IsAnimatingAbilityForAnyMesh( this ) )
        {
            ability_system_component->CurrentMontageJumpToSectionForMesh( mesh, section_name );
        }
    }
}

void UGASExtGameplayAbility::MontageSetNextSectionNameForMesh( USkeletalMeshComponent * mesh, const FName from_section_name, const FName to_section_name )
{
    check( CurrentActorInfo != nullptr );

    if ( auto * ability_system_component = Cast< UGASExtAbilitySystemComponent >( GetAbilitySystemComponentFromActorInfo_Checked() ) )
    {
        if ( ability_system_component->IsAnimatingAbilityForAnyMesh( this ) )
        {
            ability_system_component->CurrentMontageSetNextSectionNameForMesh( mesh, from_section_name, to_section_name );
        }
    }
}

void UGASExtGameplayAbility::MontageStopForMesh( USkeletalMeshComponent * mesh, const float override_blend_out_time )
{
    check( CurrentActorInfo != nullptr );

    if ( auto * ability_system_component = Cast< UGASExtAbilitySystemComponent >( GetAbilitySystemComponentFromActorInfo_Checked() ) )
    {
        // We should only stop the current montage if we are the animating ability
        if ( ability_system_component->IsAnimatingAbilityForAnyMesh( this ) )
        {
            ability_system_component->CurrentMontageStopForMesh( mesh, override_blend_out_time );
        }
    }
}

void UGASExtGameplayAbility::MontageStopForAllMeshes( const float override_blend_out_time )
{
    check( CurrentActorInfo != nullptr );

    if ( auto * ability_system_component = Cast< UGASExtAbilitySystemComponent >( GetAbilitySystemComponentFromActorInfo_Checked() ) )
    {
        if ( ability_system_component->IsAnimatingAbilityForAnyMesh( this ) )
        {
            ability_system_component->StopAllCurrentMontages( override_blend_out_time );
        }
    }
}

FGASExtGameplayEffectContainerSpec UGASExtGameplayAbility::MakeEffectContainerSpecFromEffectContainer( const FGASExtGameplayEffectContainer & effect_container, const FGameplayEventData & event_data ) const
{
    return UGASExtAbilitySystemFunctionLibrary::MakeEffectContainerSpecFromEffectContainer( this, effect_container, event_data );
}
