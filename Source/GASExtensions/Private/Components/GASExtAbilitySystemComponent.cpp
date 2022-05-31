#include "Components/GASExtAbilitySystemComponent.h"

#include "Abilities/GASExtGameplayAbility.h"
#include "Animation/GASExtAnimInstance.h"
#include "DVEDataValidator.h"

#include <AbilitySystemGlobals.h>
#include <Animation/AnimInstance.h>
#include <GameplayCueManager.h>
#include <Net/UnrealNetwork.h>

namespace
{
    UAnimInstance * GetMeshAnimInstance( USkeletalMeshComponent * mesh )
    {
        return IsValid( mesh )
                   ? mesh->GetAnimInstance()
                   : nullptr;
    }

    bool HasParentEqualToActor( const FGameplayAbilityRepAnimMontageForMesh & rep_anim_montage_info, AActor * actor )
    {
        if ( actor == nullptr )
        {
            return false;
        }

        auto * owner = rep_anim_montage_info.Mesh->GetOwner();

        while ( owner != nullptr )
        {
            if ( owner == actor )
            {
                return true;
            }

            owner = owner->GetOwner();
        }

        return false;
    }
}

static TAutoConsoleVariable< float > CVarReplayMontageErrorThreshold( TEXT( "replay.MontageErrorThresholdSW" ), 0.5f, TEXT( "Tolerance level for when montage playback position correction occurs in replays" ) );

bool FGameplayAbilityRepAnimMontageForMesh::NetSerialize( FArchive & ar, UPackageMap * map, bool & out_success )
{
    // Need to call manually, since we implement this function in the struct holding it, it won't call the function automatically anymore
    RepMontageInfo.NetSerialize( ar, map, out_success );

    ar << Mesh;

    out_success = true;
    return true;
}

UGASExtAbilitySystemComponent::UGASExtAbilitySystemComponent()
{
    bGiveAbilitiesAndEffectsInBeginPlay = true;

    FMemory::Memset( ActivationGroupCounts, 0, sizeof( ActivationGroupCounts ) );
}

void UGASExtAbilitySystemComponent::InitializeComponent()
{
    Super::InitializeComponent();

    for ( const auto & attribute_set_class : AdditionalAttributeSetClass )
    {
        auto * attribute_set = NewObject< UAttributeSet >( GetOwner(), attribute_set_class );
        AddAttributeSetSubobject( attribute_set );
    }
}

void UGASExtAbilitySystemComponent::BeginPlay()
{
    Super::BeginPlay();

    if ( bGiveAbilitiesAndEffectsInBeginPlay )
    {
        GiveDefaultAbilities();
        GiveDefaultEffects();
        GiveDefaultAttributes();
    }

    AddLooseGameplayTags( LooseTagsContainer );
}

// ReSharper disable once CppInconsistentNaming
void UGASExtAbilitySystemComponent::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const
{
    Super::GetLifetimeReplicatedProps( OutLifetimeProps );

    DOREPLIFETIME( UGASExtAbilitySystemComponent, RepAnimMontageInfoForMeshes );
}

bool UGASExtAbilitySystemComponent::GetShouldTick() const
{
    for ( const auto & rep_montage_info : RepAnimMontageInfoForMeshes )
    {
        const auto has_replicated_montage_info_to_update = ( IsOwnerActorAuthoritative() && rep_montage_info.RepMontageInfo.IsStopped == false );

        if ( has_replicated_montage_info_to_update )
        {
            return true;
        }
    }

    return Super::GetShouldTick();
}

void UGASExtAbilitySystemComponent::TickComponent( const float delta_time, const ELevelTick tick_type, FActorComponentTickFunction * this_tick_function )
{
    if ( IsOwnerActorAuthoritative() )
    {
        for ( const auto & montage_info : LocalAnimMontageInfoForMeshes )
        {
            AnimMontage_UpdateReplicatedDataForMesh( montage_info.Mesh );
        }
    }

    Super::TickComponent( delta_time, tick_type, this_tick_function );
}

void UGASExtAbilitySystemComponent::InitAbilityActorInfo( AActor * owner_actor, AActor * avatar_actor )
{
    const auto * actor_info = AbilityActorInfo.Get();
    const bool bHasNewPawnAvatar = Cast< APawn >( avatar_actor ) != nullptr && avatar_actor != actor_info->AvatarActor;

    Super::InitAbilityActorInfo( owner_actor, avatar_actor );

    LocalAnimMontageInfoForMeshes.Reset();
    RepAnimMontageInfoForMeshes.Reset();

    if ( bPendingMontageRep )
    {
        OnRep_ReplicatedAnimMontageForMesh();
    }

    if ( bHasNewPawnAvatar )
    {
        // Notify all abilities that a new pawn avatar has been set
        for ( const auto & ability_spec : ActivatableAbilities.Items )
        {
            auto * gas_ext_ability_cdo = CastChecked< UGASExtGameplayAbility >( ability_spec.Ability );

            if ( gas_ext_ability_cdo->GetInstancingPolicy() != EGameplayAbilityInstancingPolicy::NonInstanced )
            {
                const auto & instances = ability_spec.GetAbilityInstances();
                for ( UGameplayAbility * AbilityInstance : instances )
                {
                    auto * gas_ext_ability_instance = CastChecked< UGASExtGameplayAbility >( AbilityInstance );
                    gas_ext_ability_instance->OnPawnAvatarSet();
                }
            }
            else
            {
                gas_ext_ability_cdo->OnPawnAvatarSet();
            }
        }

        if ( auto * anim_instance = Cast< UGASExtAnimInstance >( actor_info->GetAnimInstance() ) )
        {
            anim_instance->InitializeWithAbilitySystem( this );
        }

        TryActivateAbilitiesOnSpawn();
    }
}

void UGASExtAbilitySystemComponent::RemoveGameplayCue_Internal( const FGameplayTag gameplay_cue_tag, FActiveGameplayCueContainer & gameplay_cue_container )
{
    // This function is overriden because of an issue in singleplayer of losing data passed to the FGameplayCueParameters of the GameplayCue.
    // This data was lost because the original implementation created a new FGameplayCueParameters if authoritative, and passed that on.
    // Only the Instigator and Effect Causer were set.
    // This meant that all other data was being lost in singleplayer, making it impossible to access that data in the OnRemove function in blueprints.
    // This wasn't a problem in multiplayer, because each client would call PredictiveRemove on the FActiveGameplayCueContainer.
    // This finds the cue in the GameplayCues array and gets the original FGameplayCueParameters from it, to then pass it along.
    // Using that method to find and pass the original FGameplayCueParameters for authoritative owners too, allows to access that data perfectly fine in the OnRemove.

    if ( IsOwnerActorAuthoritative() )
    {
        const auto was_in_list = HasMatchingGameplayTag( gameplay_cue_tag );

        if ( was_in_list )
        {
            // Instead of creating new parameters and calling InitDefaultGameplayCueParameters, find the parameters that were passed with the cue originally
            // and pass that on to the InvokeGameplayCueEvent
            for ( auto idx = 0; idx < gameplay_cue_container.GameplayCues.Num(); ++idx )
            {
                auto & cue = gameplay_cue_container.GameplayCues[ idx ];
                if ( cue.GameplayCueTag == gameplay_cue_tag )
                {
                    InvokeGameplayCueEvent( gameplay_cue_tag, EGameplayCueEvent::Removed, cue.Parameters );
                    break;
                }
            }
        }

        gameplay_cue_container.RemoveCue( gameplay_cue_tag );
    }
    else if ( ScopedPredictionKey.IsLocalClientKey() )
    {
        gameplay_cue_container.PredictiveRemove( gameplay_cue_tag );
    }
}

#if WITH_EDITOR
EDataValidationResult UGASExtAbilitySystemComponent::IsDataValid( TArray< FText > & validation_errors )
{
    Super::IsDataValid( validation_errors );

    return FDVEDataValidator( validation_errors )
        .NoNullItem( VALIDATOR_GET_PROPERTY( DefaultEffects ) )
        .NoNullItem( VALIDATOR_GET_PROPERTY( DefaultAbilities ) )
        .Result();
}
#endif

FGameplayAbilitySpecHandle UGASExtAbilitySystemComponent::FindAbilitySpecHandleForClass( const TSubclassOf< UGameplayAbility > & ability_class )
{
    ABILITYLIST_SCOPE_LOCK();

    for ( const auto & ability_spec : ActivatableAbilities.Items )
    {
        if ( ability_spec.Ability->GetClass() == ability_class )
        {
            return ability_spec.Handle;
        }
    }

    return FGameplayAbilitySpecHandle();
}

void UGASExtAbilitySystemComponent::OurCancelAllAbilities()
{
    static const auto GameplayTagContainer = FGameplayTagContainer::CreateFromArray(
        TArray< FGameplayTag >( { FGameplayTag::RequestGameplayTag( "Ability" ) } ) );

    CancelAbilities( &GameplayTagContainer );
}

// ReSharper disable CppMemberFunctionMayBeConst
void UGASExtAbilitySystemComponent::ExecuteGameplayCueLocal( const FGameplayTag gameplay_cue_tag, const FGameplayCueParameters & parameters )
{
    UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue( GetOwner(), gameplay_cue_tag, EGameplayCueEvent::Type::Executed, parameters );
}

void UGASExtAbilitySystemComponent::AddGameplayCueLocal( const FGameplayTag gameplay_cue_tag, const FGameplayCueParameters & parameters )
{
    UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue( GetOwner(), gameplay_cue_tag, EGameplayCueEvent::Type::OnActive, parameters );
}

void UGASExtAbilitySystemComponent::RemoveGameplayCueLocal( const FGameplayTag gameplay_cue_tag, const FGameplayCueParameters & parameters )
{
    UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue( GetOwner(), gameplay_cue_tag, EGameplayCueEvent::Type::Removed, parameters );
}
// ReSharper restore CppMemberFunctionMayBeConst

float UGASExtAbilitySystemComponent::PlayMontageForMesh( UGameplayAbility * animating_ability, USkeletalMeshComponent * mesh, UAnimMontage * new_anim_montage, const float play_rate, const FName start_section_name, const bool must_replicate_montage )
{
    auto * ability = Cast< UGASExtGameplayAbility >( animating_ability );

    auto duration = -1.f;

    auto * anim_instance = GetMeshAnimInstance( mesh );

    if ( anim_instance != nullptr && new_anim_montage != nullptr )
    {
        duration = anim_instance->Montage_Play( new_anim_montage, play_rate );
        if ( duration > 0.f )
        {
            auto & anim_montage_info = GetLocalAnimMontageInfoForMesh( mesh );

            if ( anim_montage_info.LocalMontageInfo.AnimatingAbility != nullptr && anim_montage_info.LocalMontageInfo.AnimatingAbility != animating_ability )
            {
                // The ability that was previously animating will have already gotten the 'interrupted' callback.
                // It may be a good idea to make this a global policy and 'cancel' the ability.
                //
                // For now, we expect it to end itself when this happens.
            }

            if ( new_anim_montage->HasRootMotion() && anim_instance->GetOwningActor() )
            {
                UE_LOG( LogRootMotion, Log, TEXT( "UAbilitySystemComponent::PlayMontage %s, Role: %s" ), *GetNameSafe( new_anim_montage ), *UEnum::GetValueAsString( TEXT( "Engine.ENetRole" ), anim_instance->GetOwningActor()->GetLocalRole() ) );
            }

            anim_montage_info.LocalMontageInfo.AnimMontage = new_anim_montage;
            anim_montage_info.LocalMontageInfo.AnimatingAbility = animating_ability;
            anim_montage_info.LocalMontageInfo.PlayBit = !anim_montage_info.LocalMontageInfo.PlayBit;

            if ( ability != nullptr )
            {
                ability->SetCurrentMontageForMesh( mesh, new_anim_montage );
            }

            // Start at a given Section.
            if ( start_section_name != NAME_None )
            {
                anim_instance->Montage_JumpToSection( start_section_name, new_anim_montage );
            }

            // Replicate to non owners
            if ( IsOwnerActorAuthoritative() )
            {
                if ( must_replicate_montage )
                {
                    // Those are static parameters, they are only set when the montage is played. They are not changed after that.
                    auto & ability_rep_montage_info = GetGameplayAbilityRepAnimMontageForMesh( mesh );
                    ability_rep_montage_info.RepMontageInfo.AnimMontage = new_anim_montage;
                    ability_rep_montage_info.RepMontageInfo.ForcePlayBit = !static_cast< bool >( ability_rep_montage_info.RepMontageInfo.ForcePlayBit );

                    // Update parameters that change during Montage life time.
                    AnimMontage_UpdateReplicatedDataForMesh( mesh );

                    // Force net update on our avatar actor
                    if ( AbilityActorInfo->AvatarActor != nullptr )
                    {
                        AbilityActorInfo->AvatarActor->ForceNetUpdate();
                    }
                }
            }
            else
            {
                // If this prediction key is rejected, we need to end the preview
                auto prediction_key = GetPredictionKeyForNewAction();
                if ( prediction_key.IsValidKey() )
                {
                    prediction_key.NewRejectedDelegate().BindUObject( this, &UGASExtAbilitySystemComponent::OnPredictiveMontageRejectedForMesh, mesh, new_anim_montage );
                }
            }
        }
    }

    return duration;
}

float UGASExtAbilitySystemComponent::PlayMontageSimulatedForMesh( USkeletalMeshComponent * mesh, UAnimMontage * new_anim_montage, const float play_rate, FName start_section_name )
{
    auto duration = -1.f;
    auto * anim_instance = GetMeshAnimInstance( mesh );

    if ( anim_instance != nullptr && new_anim_montage != nullptr )
    {
        duration = anim_instance->Montage_Play( new_anim_montage, play_rate );
        if ( duration > 0.f )
        {
            auto & anim_montage_info = GetLocalAnimMontageInfoForMesh( mesh );
            anim_montage_info.LocalMontageInfo.AnimMontage = new_anim_montage;
        }
    }

    return duration;
}

void UGASExtAbilitySystemComponent::CurrentMontageStopForMesh( USkeletalMeshComponent * mesh, const float override_blend_out_time )
{
    auto * anim_instance = GetMeshAnimInstance( mesh );

    auto & anim_montage_info = GetLocalAnimMontageInfoForMesh( mesh );
    auto * montage_to_stop = anim_montage_info.LocalMontageInfo.AnimMontage;
    const auto should_stop_montage = anim_instance && montage_to_stop && !anim_instance->Montage_GetIsStopped( montage_to_stop );

    if ( should_stop_montage )
    {
        const auto blend_out_time = ( override_blend_out_time >= 0.0f
                                          ? override_blend_out_time
                                          : montage_to_stop->BlendOut.GetBlendTime() );

        anim_instance->Montage_Stop( blend_out_time, montage_to_stop );

        if ( IsOwnerActorAuthoritative() )
        {
            AnimMontage_UpdateReplicatedDataForMesh( mesh );
        }
    }
}

void UGASExtAbilitySystemComponent::StopAllCurrentMontages( const float override_blend_out_time )
{
    for ( const auto & gameplay_ability_local_anim_montage_for_mesh : LocalAnimMontageInfoForMeshes )
    {
        CurrentMontageStopForMesh( gameplay_ability_local_anim_montage_for_mesh.Mesh, override_blend_out_time );
    }
}

void UGASExtAbilitySystemComponent::StopMontageIfCurrentForMesh( USkeletalMeshComponent * mesh, const UAnimMontage & montage, const float override_blend_out_time )
{
    const auto & anim_montage_info = GetLocalAnimMontageInfoForMesh( mesh );
    if ( &montage == anim_montage_info.LocalMontageInfo.AnimMontage )
    {
        CurrentMontageStopForMesh( mesh, override_blend_out_time );
    }
}

void UGASExtAbilitySystemComponent::ClearAnimatingAbilityForAllMeshes( UGameplayAbility * ability )
{
    auto * sw_ability = Cast< UGASExtGameplayAbility >( ability );
    for ( auto & gameplay_ability_local_anim_montage_for_mesh : LocalAnimMontageInfoForMeshes )
    {
        if ( gameplay_ability_local_anim_montage_for_mesh.LocalMontageInfo.AnimatingAbility == ability )
        {
            sw_ability->SetCurrentMontageForMesh( gameplay_ability_local_anim_montage_for_mesh.Mesh, nullptr );
            gameplay_ability_local_anim_montage_for_mesh.LocalMontageInfo.AnimatingAbility = nullptr;
        }
    }
}

void UGASExtAbilitySystemComponent::CurrentMontageJumpToSectionForMesh( USkeletalMeshComponent * mesh, const FName section_name )
{
    auto * anim_instance = GetMeshAnimInstance( mesh );

    auto & anim_montage_info = GetLocalAnimMontageInfoForMesh( mesh );

    if ( ( section_name != NAME_None ) && anim_instance != nullptr && anim_montage_info.LocalMontageInfo.AnimMontage != nullptr )
    {
        anim_instance->Montage_JumpToSection( section_name, anim_montage_info.LocalMontageInfo.AnimMontage );
        if ( IsOwnerActorAuthoritative() )
        {
            AnimMontage_UpdateReplicatedDataForMesh( mesh );
        }
        else
        {
            ServerCurrentMontageJumpToSectionNameForMesh( mesh, anim_montage_info.LocalMontageInfo.AnimMontage, section_name );
        }
    }
}

void UGASExtAbilitySystemComponent::CurrentMontageSetNextSectionNameForMesh( USkeletalMeshComponent * mesh, const FName from_section_name, const FName to_section_name )
{
    auto * anim_instance = GetMeshAnimInstance( mesh );

    const auto & anim_montage_info = GetLocalAnimMontageInfoForMesh( mesh );
    if ( anim_montage_info.LocalMontageInfo.AnimMontage != nullptr && anim_instance != nullptr )
    {
        // Set Next Section Name.
        anim_instance->Montage_SetNextSection( from_section_name, to_section_name, anim_montage_info.LocalMontageInfo.AnimMontage );

        // Update replicated version for Simulated Proxies if we are on the server.
        if ( IsOwnerActorAuthoritative() )
        {
            AnimMontage_UpdateReplicatedDataForMesh( mesh );
        }
        else
        {
            const auto current_position = anim_instance->Montage_GetPosition( anim_montage_info.LocalMontageInfo.AnimMontage );
            ServerCurrentMontageSetNextSectionNameForMesh( mesh, anim_montage_info.LocalMontageInfo.AnimMontage, current_position, from_section_name, to_section_name );
        }
    }
}

void UGASExtAbilitySystemComponent::CurrentMontageSetPlayRateForMesh( USkeletalMeshComponent * mesh, const float play_rate )
{
    auto * anim_instance = GetMeshAnimInstance( mesh );
    const auto & anim_montage_info = GetLocalAnimMontageInfoForMesh( mesh );
    if ( anim_montage_info.LocalMontageInfo.AnimMontage != nullptr && anim_instance != nullptr )
    {
        // Set Play Rate
        anim_instance->Montage_SetPlayRate( anim_montage_info.LocalMontageInfo.AnimMontage, play_rate );

        // Update replicated version for Simulated Proxies if we are on the server.
        if ( IsOwnerActorAuthoritative() )
        {
            AnimMontage_UpdateReplicatedDataForMesh( mesh );
        }
        else
        {
            ServerCurrentMontageSetPlayRateForMesh( mesh, anim_montage_info.LocalMontageInfo.AnimMontage, play_rate );
        }
    }
}

bool UGASExtAbilitySystemComponent::IsAnimatingAbilityForAnyMesh( UGameplayAbility * ability ) const
{
    for ( const auto & gameplay_ability_local_anim_montage_for_mesh : LocalAnimMontageInfoForMeshes )
    {
        if ( gameplay_ability_local_anim_montage_for_mesh.LocalMontageInfo.AnimatingAbility == ability )
        {
            return true;
        }
    }

    return false;
}

UGameplayAbility * UGASExtAbilitySystemComponent::GetAnimatingAbilityFromAnyMesh() const
{
    // Only one ability can be animating for all meshes
    for ( const auto & gameplay_ability_local_anim_montage_for_mesh : LocalAnimMontageInfoForMeshes )
    {
        if ( gameplay_ability_local_anim_montage_for_mesh.LocalMontageInfo.AnimatingAbility != nullptr )
        {
            return gameplay_ability_local_anim_montage_for_mesh.LocalMontageInfo.AnimatingAbility;
        }
    }

    return nullptr;
}

TArray< UAnimMontage * > UGASExtAbilitySystemComponent::GetCurrentMontages() const
{
    TArray< UAnimMontage * > montages;

    for ( const auto & gameplay_ability_local_anim_montage_for_mesh : LocalAnimMontageInfoForMeshes )
    {
        const auto * anim_instance = IsValid( gameplay_ability_local_anim_montage_for_mesh.Mesh ) && gameplay_ability_local_anim_montage_for_mesh.Mesh->GetOwner() == AbilityActorInfo->AvatarActor
                                         ? gameplay_ability_local_anim_montage_for_mesh.Mesh->GetAnimInstance()
                                         : nullptr;

        if ( gameplay_ability_local_anim_montage_for_mesh.LocalMontageInfo.AnimMontage && anim_instance != nullptr && anim_instance->Montage_IsActive( gameplay_ability_local_anim_montage_for_mesh.LocalMontageInfo.AnimMontage ) )
        {
            montages.Add( gameplay_ability_local_anim_montage_for_mesh.LocalMontageInfo.AnimMontage );
        }
    }

    return montages;
}

UAnimMontage * UGASExtAbilitySystemComponent::GetCurrentMontageForMesh( USkeletalMeshComponent * mesh ) const
{
    auto * anim_instance = GetMeshAnimInstance( mesh );
    auto & anim_montage_info = const_cast< UGASExtAbilitySystemComponent * >( this )->GetLocalAnimMontageInfoForMesh( mesh );

    if ( anim_montage_info.LocalMontageInfo.AnimMontage && anim_instance != nullptr && anim_instance->Montage_IsActive( anim_montage_info.LocalMontageInfo.AnimMontage ) )
    {
        return anim_montage_info.LocalMontageInfo.AnimMontage;
    }

    return nullptr;
}

int32 UGASExtAbilitySystemComponent::GetCurrentMontageSectionIDForMesh( USkeletalMeshComponent * mesh ) const
{
    auto * anim_instance = GetMeshAnimInstance( mesh );
    auto * current_anim_montage = GetCurrentMontageForMesh( mesh );

    if ( current_anim_montage != nullptr && anim_instance != nullptr )
    {
        const auto montage_position = anim_instance->Montage_GetPosition( current_anim_montage );
        return current_anim_montage->GetSectionIndexFromPosition( montage_position );
    }

    return INDEX_NONE;
}

FName UGASExtAbilitySystemComponent::GetCurrentMontageSectionNameForMesh( USkeletalMeshComponent * mesh ) const
{
    auto * anim_instance = GetMeshAnimInstance( mesh );
    auto * current_anim_montage = GetCurrentMontageForMesh( mesh );

    if ( current_anim_montage && anim_instance )
    {
        const auto montage_position = anim_instance->Montage_GetPosition( current_anim_montage );
        const auto current_section_id = current_anim_montage->GetSectionIndexFromPosition( montage_position );

        return current_anim_montage->GetSectionName( current_section_id );
    }

    return NAME_None;
}

float UGASExtAbilitySystemComponent::GetCurrentMontageSectionLengthForMesh( USkeletalMeshComponent * mesh ) const
{
    auto * anim_instance = GetMeshAnimInstance( mesh );
    auto * current_anim_montage = GetCurrentMontageForMesh( mesh );

    if ( current_anim_montage && anim_instance != nullptr )
    {
        const auto current_section_id = GetCurrentMontageSectionIDForMesh( mesh );
        if ( current_section_id != INDEX_NONE )
        {
            auto & composite_sections = current_anim_montage->CompositeSections;

            // If we have another section after us, then take delta between both start times.
            if ( current_section_id < ( composite_sections.Num() - 1 ) )
            {
                return ( composite_sections[ current_section_id + 1 ].GetTime() - composite_sections[ current_section_id ].GetTime() );
            }
            // Otherwise we are the last section, so take delta with Montage total time.

            return ( current_anim_montage->SequenceLength - composite_sections[ current_section_id ].GetTime() );
        }

        // if we have no sections, just return total length of Montage.
        return current_anim_montage->SequenceLength;
    }

    return 0.0f;
}

float UGASExtAbilitySystemComponent::GetCurrentMontageSectionTimeLeftForMesh( USkeletalMeshComponent * mesh ) const
{
    auto * anim_instance = GetMeshAnimInstance( mesh );
    auto * current_anim_montage = GetCurrentMontageForMesh( mesh );

    if ( current_anim_montage && anim_instance && anim_instance->Montage_IsActive( current_anim_montage ) )
    {
        const auto current_position = anim_instance->Montage_GetPosition( current_anim_montage );
        return current_anim_montage->GetSectionTimeLeftFromPos( current_position );
    }

    return -1.f;
}

void UGASExtAbilitySystemComponent::GiveDefaultAbilities()
{
    if ( !GetOwner()->HasAuthority() )
    {
        return;
    }

    for ( const auto & startup_ability : DefaultAbilities )
    {
        if ( !ensureAlwaysMsgf( startup_ability != nullptr, TEXT( "%s() One of the DefaultAbilities is not valid in %s." ), TEXT( __FUNCTION__ ), *GetName() ) )
        {
            continue;
        }

        GiveAbility( FGameplayAbilitySpec(
            startup_ability,
            1,
            startup_ability->GetDefaultObject< UGASExtGameplayAbility >()->GetInputID(),
            this ) );
    }
}

void UGASExtAbilitySystemComponent::GiveDefaultEffects()
{
    if ( !GetOwner()->HasAuthority() )
    {
        return;
    }

    auto effect_context = MakeEffectContext();
    effect_context.AddSourceObject( this );

    for ( const auto & startup_effect : DefaultEffects )
    {
        if ( !ensureAlwaysMsgf( startup_effect != nullptr, TEXT( "%s() One of the StartupEffects is not valid in %s." ), TEXT( __FUNCTION__ ), *GetName() ) )
        {
            continue;
        }

        const auto new_handle = MakeOutgoingSpec( startup_effect, 1, effect_context );

        if ( new_handle.IsValid() )
        {
            ApplyGameplayEffectSpecToTarget( *new_handle.Data.Get(), this );
        }
    }
}

void UGASExtAbilitySystemComponent::GiveDefaultAttributes()
{
    if ( DefaultAttributes == nullptr )
    {
        UE_LOG( LogTemp, Verbose, TEXT( "%s() Missing DefaultAttributes for %s." ), TEXT( __FUNCTION__ ), *GetNameSafe( GetOwner() ) );
        return;
    }

    // Can run on Server and Client
    auto effect_context = MakeEffectContext();
    effect_context.AddSourceObject( this );

    const auto new_handle = MakeOutgoingSpec( DefaultAttributes, 1, effect_context );

    if ( new_handle.IsValid() )
    {
        ApplyGameplayEffectSpecToSelf( *new_handle.Data.Get() );
    }
}

bool UGASExtAbilitySystemComponent::IsActivationGroupBlocked( EGASExtAbilityActivationGroup group ) const
{
    auto is_blocked = false;

    switch ( group )
    {
        case EGASExtAbilityActivationGroup::Independent:
        {
            // Independent abilities are never blocked.
            is_blocked = false;
        }
        break;

        case EGASExtAbilityActivationGroup::ExclusiveReplaceable:
        case EGASExtAbilityActivationGroup::ExclusiveBlocking:
        {
            // Exclusive abilities can activate if nothing is blocking.
            is_blocked = ( ActivationGroupCounts[ static_cast< uint8 >( EGASExtAbilityActivationGroup::ExclusiveBlocking ) ] > 0 );
        }
        break;

        default:
        {
            checkf( false, TEXT( "IsActivationGroupBlocked: Invalid ActivationGroup [%d]\n" ), static_cast< uint8 >( group ) );
        }
        break;
    }

    return is_blocked;
}

void UGASExtAbilitySystemComponent::AddAbilityToActivationGroup( EGASExtAbilityActivationGroup group, UGASExtGameplayAbility * ability )
{
    check( ability );
    check( ActivationGroupCounts[ static_cast< uint8 >( group ) ] < INT32_MAX );

    ActivationGroupCounts[ static_cast< uint8 >( group ) ]++;

    switch ( group )
    {
        case EGASExtAbilityActivationGroup::Independent:
        {
            // Independent abilities do not cancel any other abilities.
        }
        break;

        case EGASExtAbilityActivationGroup::ExclusiveReplaceable:
        case EGASExtAbilityActivationGroup::ExclusiveBlocking:
        {
            CancelActivationGroupAbilities( EGASExtAbilityActivationGroup::ExclusiveReplaceable, ability, false );
        }
        break;

        default:
        {
            checkf( false, TEXT( "AddAbilityToActivationGroup: Invalid ActivationGroup [%d]\n" ), static_cast< uint8 >( group ) );
        }
        break;
    }

    const auto exclusive_count = ActivationGroupCounts[ static_cast< uint8 >( EGASExtAbilityActivationGroup::ExclusiveReplaceable ) ] + ActivationGroupCounts[ static_cast< uint8 >( EGASExtAbilityActivationGroup::ExclusiveBlocking ) ];
    if ( !ensure( exclusive_count <= 1 ) )
    {
        UE_LOG( LogTemp, Error, TEXT( "AddAbilityToActivationGroup: Multiple exclusive abilities are running." ) );
    }
}

void UGASExtAbilitySystemComponent::RemoveAbilityFromActivationGroup( const EGASExtAbilityActivationGroup group, const UGASExtGameplayAbility * ability )
{
    check( ability );
    check( ActivationGroupCounts[ static_cast< uint8 >( group ) ] > 0 );
    check( group < EGASExtAbilityActivationGroup::MAX );

    ActivationGroupCounts[ static_cast< uint8 >( group ) ]--;
}

void UGASExtAbilitySystemComponent::CancelActivationGroupAbilities( const EGASExtAbilityActivationGroup group, UGASExtGameplayAbility * ignore_ability, bool replicate_cancel_ability )
{
    const auto should_cancel_func = [ this, group, ignore_ability ]( const UGASExtGameplayAbility * ability, FGameplayAbilitySpecHandle ability_handle ) {
        return ( ( ability->GetActivationGroup() == group ) && ( ability != ignore_ability ) );
    };

    CancelAbilitiesByFunc( should_cancel_func, replicate_cancel_ability );
}

void UGASExtAbilitySystemComponent::CancelAbilitiesByFunc( TShouldCancelAbilityFunc predicate, bool replicate_cancel_ability )
{
    ABILITYLIST_SCOPE_LOCK();
    for ( const FGameplayAbilitySpec & ability_spec : ActivatableAbilities.Items )
    {
        if ( !ability_spec.IsActive() )
        {
            continue;
        }

        auto * ability_cdo = CastChecked< UGASExtGameplayAbility >( ability_spec.Ability );

        if ( ability_cdo->GetInstancingPolicy() != EGameplayAbilityInstancingPolicy::NonInstanced )
        {
            // Cancel all the spawned instances, not the CDO.
            const auto & instances = ability_spec.GetAbilityInstances();
            for ( auto * ability_instance : instances )
            {
                auto * gas_ext_ability_instance = CastChecked< UGASExtGameplayAbility >( ability_instance );

                if ( predicate( gas_ext_ability_instance, ability_spec.Handle ) )
                {
                    if ( gas_ext_ability_instance->CanBeCanceled() )
                    {
                        gas_ext_ability_instance->CancelAbility( ability_spec.Handle, AbilityActorInfo.Get(), gas_ext_ability_instance->GetCurrentActivationInfo(), replicate_cancel_ability );
                    }
                    else
                    {
                        UE_LOG( LogTemp, Error, TEXT( "CancelAbilitiesByFunc: Can't cancel ability [%s] because CanBeCanceled is false." ), *gas_ext_ability_instance->GetName() );
                    }
                }
            }
        }
        else
        {
            // Cancel the non-instanced ability CDO.
            if ( predicate( ability_cdo, ability_spec.Handle ) )
            {
                // Non-instanced abilities can always be canceled.
                check( ability_cdo->CanBeCanceled() );
                ability_cdo->CancelAbility( ability_spec.Handle, AbilityActorInfo.Get(), FGameplayAbilityActivationInfo(), replicate_cancel_ability );
            }
        }
    }
}

void UGASExtAbilitySystemComponent::CancelInputActivatedAbilities( bool replicate_cancel_ability )
{
    const TShouldCancelAbilityFunc predicate = [ this ]( const UGASExtGameplayAbility * ability, FGameplayAbilitySpecHandle handle ) {
        const auto activation_policy = ability->GetActivationPolicy();
        return activation_policy == EGASExtAbilityActivationPolicy::OnInputTriggered || activation_policy == EGASExtAbilityActivationPolicy::WhileInputActive;
    };

    CancelAbilitiesByFunc( predicate, replicate_cancel_ability );
}

void UGASExtAbilitySystemComponent::TryActivateAbilitiesOnSpawn()
{
    ABILITYLIST_SCOPE_LOCK();
    for ( const FGameplayAbilitySpec & ability_spec : ActivatableAbilities.Items )
    {
        const auto * ability_cdo = CastChecked< UGASExtGameplayAbility >( ability_spec.Ability );
        ability_cdo->TryActivateAbilityOnSpawn( AbilityActorInfo.Get(), ability_spec );
    }
}

void UGASExtAbilitySystemComponent::NotifyAbilityActivated( const FGameplayAbilitySpecHandle Handle, UGameplayAbility * Ability )
{
    Super::NotifyAbilityActivated( Handle, Ability );

    auto * ability = CastChecked< UGASExtGameplayAbility >( Ability );

    AddAbilityToActivationGroup( ability->GetActivationGroup(), ability );
}

void UGASExtAbilitySystemComponent::NotifyAbilityFailed( const FGameplayAbilitySpecHandle Handle, UGameplayAbility * Ability, const FGameplayTagContainer & FailureReason )
{
    Super::NotifyAbilityFailed( Handle, Ability, FailureReason );

    /* :TODO: Failed abilities
    if ( APawn * Avatar = Cast< APawn >( GetAvatarActor() ) )
    {
        if ( !Avatar->IsLocallyControlled() && Ability->IsSupportedForNetworking() )
        {
            ClientNotifyAbilityFailed( Ability, FailureReason );
            return;
        }
    }

    HandleAbilityFailed( Ability, FailureReason );*/
}

void UGASExtAbilitySystemComponent::NotifyAbilityEnded( const FGameplayAbilitySpecHandle handle, UGameplayAbility * ability, const bool was_cancelled )
{
    Super::NotifyAbilityEnded( handle, ability, was_cancelled );

    // If AnimatingAbility ended, clear the pointer
    ClearAnimatingAbilityForAllMeshes( ability );

    const auto * gas_ext_ability = CastChecked< UGASExtGameplayAbility >( ability );

    RemoveAbilityFromActivationGroup( gas_ext_ability->GetActivationGroup(), gas_ext_ability );
}

void UGASExtAbilitySystemComponent::K2_RemoveGameplayCue( const FGameplayTag gameplay_cue_tag )
{
    RemoveGameplayCue( gameplay_cue_tag );
}

void UGASExtAbilitySystemComponent::K2_ExecuteGameplayCueWithEffectContext( const FGameplayTag gameplay_cue_tag, const FGameplayEffectContextHandle effect_context )
{
    ExecuteGameplayCue( gameplay_cue_tag, effect_context );
}

void UGASExtAbilitySystemComponent::K2_ExecuteGameplayCueWithParameters( const FGameplayTag gameplay_cue_tag, const FGameplayCueParameters & parameters )
{
    ExecuteGameplayCue( gameplay_cue_tag, parameters );
}

void UGASExtAbilitySystemComponent::K2_AddGameplayCueWithEffectContext( const FGameplayTag gameplay_cue_tag, const FGameplayEffectContextHandle effect_context )
{
    AddGameplayCue( gameplay_cue_tag, effect_context );
}

void UGASExtAbilitySystemComponent::K2_AddGameplayCueWithParameters( const FGameplayTag gameplay_cue_tag, const FGameplayCueParameters & parameters )
{
    AddGameplayCue( gameplay_cue_tag, parameters );
}

FGameplayAbilityLocalAnimMontageForMesh & UGASExtAbilitySystemComponent::GetLocalAnimMontageInfoForMesh( USkeletalMeshComponent * mesh )
{
    if ( auto * montage_info = LocalAnimMontageInfoForMeshes.FindByPredicate( [ mesh ]( const auto & item ) {
             return item.Mesh == mesh;
         } ) )
    {
        return *montage_info;
    }

    const auto montage_info = FGameplayAbilityLocalAnimMontageForMesh( mesh );
    return LocalAnimMontageInfoForMeshes.Add_GetRef( montage_info );
}

FGameplayAbilityRepAnimMontageForMesh & UGASExtAbilitySystemComponent::GetGameplayAbilityRepAnimMontageForMesh( USkeletalMeshComponent * mesh )
{
    if ( auto * rep_montage_info = RepAnimMontageInfoForMeshes.FindByPredicate( [ mesh ]( const auto & item ) {
             return item.Mesh == mesh;
         } ) )
    {
        return *rep_montage_info;
    }

    const auto rep_montage_info = FGameplayAbilityRepAnimMontageForMesh( mesh );
    return RepAnimMontageInfoForMeshes.Add_GetRef( rep_montage_info );
}

void UGASExtAbilitySystemComponent::OnPredictiveMontageRejectedForMesh( USkeletalMeshComponent * mesh, UAnimMontage * predictive_montage ) const
{
    static const auto montage_prediction_reject_fade_time = 0.25f;

    auto * anim_instance = GetMeshAnimInstance( mesh );

    if ( anim_instance != nullptr && predictive_montage != nullptr )
    {
        // If this montage is still playing: kill it
        if ( anim_instance->Montage_IsPlaying( predictive_montage ) )
        {
            anim_instance->Montage_Stop( montage_prediction_reject_fade_time, predictive_montage );
        }
    }
}

void UGASExtAbilitySystemComponent::AnimMontage_UpdateReplicatedDataForMesh( USkeletalMeshComponent * mesh )
{
    check( IsOwnerActorAuthoritative() );

    AnimMontage_UpdateReplicatedDataForMesh( GetGameplayAbilityRepAnimMontageForMesh( mesh ) );
}

void UGASExtAbilitySystemComponent::AnimMontage_UpdateReplicatedDataForMesh( FGameplayAbilityRepAnimMontageForMesh & rep_anim_montage_info )
{
    auto * anim_instance = IsValid( rep_anim_montage_info.Mesh ) && HasParentEqualToActor( rep_anim_montage_info, AbilityActorInfo->AvatarActor.Get() )
                               ? rep_anim_montage_info.Mesh->GetAnimInstance()
                               : nullptr;

    auto & anim_montage_info = GetLocalAnimMontageInfoForMesh( rep_anim_montage_info.Mesh );

    if ( anim_instance != nullptr && anim_montage_info.LocalMontageInfo.AnimMontage )
    {
        rep_anim_montage_info.RepMontageInfo.AnimMontage = anim_montage_info.LocalMontageInfo.AnimMontage;

        // Compressed Flags
        const auto is_stopped = anim_instance->Montage_GetIsStopped( anim_montage_info.LocalMontageInfo.AnimMontage );

        if ( !is_stopped )
        {
            rep_anim_montage_info.RepMontageInfo.PlayRate = anim_instance->Montage_GetPlayRate( anim_montage_info.LocalMontageInfo.AnimMontage );
            rep_anim_montage_info.RepMontageInfo.Position = anim_instance->Montage_GetPosition( anim_montage_info.LocalMontageInfo.AnimMontage );
            rep_anim_montage_info.RepMontageInfo.BlendTime = anim_instance->Montage_GetBlendTime( anim_montage_info.LocalMontageInfo.AnimMontage );
        }

        if ( rep_anim_montage_info.RepMontageInfo.IsStopped != is_stopped )
        {
            // Set this prior to calling UpdateShouldTick, so we start ticking if we are playing a Montage
            rep_anim_montage_info.RepMontageInfo.IsStopped = is_stopped;

            // When we start or stop an animation, update the clients right away for the Avatar Actor
            if ( AbilityActorInfo->AvatarActor != nullptr )
            {
                AbilityActorInfo->AvatarActor->ForceNetUpdate();
            }

            // When this changes, we should update whether or not we should be ticking
            UpdateShouldTick();
        }

        // Replicate NextSectionID to keep it in sync.
        // We actually replicate NextSectionID+1 on a BYTE to put INDEX_NONE in there.
        const auto current_section_id = anim_montage_info.LocalMontageInfo.AnimMontage->GetSectionIndexFromPosition( rep_anim_montage_info.RepMontageInfo.Position );
        if ( current_section_id != INDEX_NONE )
        {
            const auto next_section_id = anim_instance->Montage_GetNextSectionID( anim_montage_info.LocalMontageInfo.AnimMontage, current_section_id );
            if ( next_section_id >= ( 256 - 1 ) )
            {
                ABILITY_LOG( Error, TEXT( "AnimMontage_UpdateReplicatedData. NextSectionID = %d.  RepAnimMontageInfo.Position: %.2f, CurrentSectionID: %d. LocalAnimMontageInfo.AnimMontage %s" ), next_section_id, rep_anim_montage_info.RepMontageInfo.Position, current_section_id, *GetNameSafe( anim_montage_info.LocalMontageInfo.AnimMontage ) );
                ensure( next_section_id < ( 256 - 1 ) );
            }
            rep_anim_montage_info.RepMontageInfo.NextSectionID = static_cast< uint8 >( next_section_id + 1 );
        }
        else
        {
            rep_anim_montage_info.RepMontageInfo.NextSectionID = 0;
        }
    }
}

void UGASExtAbilitySystemComponent::AnimMontage_UpdateForcedPlayFlagsForMesh( FGameplayAbilityRepAnimMontageForMesh & rep_anim_montage_info )
{
    const auto & anim_montage_info = GetLocalAnimMontageInfoForMesh( rep_anim_montage_info.Mesh );

    rep_anim_montage_info.RepMontageInfo.ForcePlayBit = anim_montage_info.LocalMontageInfo.PlayBit;
}

void UGASExtAbilitySystemComponent::OnRep_ReplicatedAnimMontageForMesh()
{
    for ( auto & new_rep_montage_info_for_mesh : RepAnimMontageInfoForMeshes )
    {
        auto & anim_montage_info = GetLocalAnimMontageInfoForMesh( new_rep_montage_info_for_mesh.Mesh );

        auto * world = GetWorld();

        if ( new_rep_montage_info_for_mesh.RepMontageInfo.bSkipPlayRate )
        {
            new_rep_montage_info_for_mesh.RepMontageInfo.PlayRate = 1.f;
        }

        const auto is_playing_replay = world && world->IsPlayingReplay();

        const auto montage_rep_pos_err_threshold = is_playing_replay
                                                       ? CVarReplayMontageErrorThreshold.GetValueOnGameThread()
                                                       : 0.1f;

        auto * anim_instance = IsValid( new_rep_montage_info_for_mesh.Mesh ) && HasParentEqualToActor( new_rep_montage_info_for_mesh, AbilityActorInfo->AvatarActor.Get() )
                                   ? new_rep_montage_info_for_mesh.Mesh->GetAnimInstance()
                                   : nullptr;
        if ( anim_instance == nullptr || !IsReadyForReplicatedMontageForMesh() )
        {
            // We can't handle this yet
            bPendingMontageRep = true;
            return;
        }
        bPendingMontageRep = false;

        if ( !AbilityActorInfo->IsLocallyControlled() )
        {
            static const auto c_var = IConsoleManager::Get().FindTConsoleVariableDataInt( TEXT( "net.Montage.Debug" ) );
            const auto must_debug_montage = ( c_var && c_var->GetValueOnGameThread() == 1 );
            if ( must_debug_montage )
            {
                ABILITY_LOG( Warning, TEXT( "\n\nOnRep_ReplicatedAnimMontage, %s" ), *GetNameSafe( this ) );
                ABILITY_LOG( Warning, TEXT( "\tAnimMontage: %s\n\tPlayRate: %f\n\tPosition: %f\n\tBlendTime: %f\n\tNextSectionID: %d\n\tIsStopped: %d\n\tForcePlayBit: %d" ), *GetNameSafe( new_rep_montage_info_for_mesh.RepMontageInfo.AnimMontage ), new_rep_montage_info_for_mesh.RepMontageInfo.PlayRate, new_rep_montage_info_for_mesh.RepMontageInfo.Position, new_rep_montage_info_for_mesh.RepMontageInfo.BlendTime, new_rep_montage_info_for_mesh.RepMontageInfo.NextSectionID, new_rep_montage_info_for_mesh.RepMontageInfo.IsStopped, new_rep_montage_info_for_mesh.RepMontageInfo.ForcePlayBit );
                ABILITY_LOG( Warning, TEXT( "\tLocalAnimMontageInfo.AnimMontage: %s\n\tPosition: %f" ), *GetNameSafe( anim_montage_info.LocalMontageInfo.AnimMontage ), anim_instance->Montage_GetPosition( anim_montage_info.LocalMontageInfo.AnimMontage ) );
            }

            if ( new_rep_montage_info_for_mesh.RepMontageInfo.AnimMontage )
            {
                // New Montage to play
                const auto replicated_play_bit = static_cast< bool >( new_rep_montage_info_for_mesh.RepMontageInfo.ForcePlayBit );
                if ( ( anim_montage_info.LocalMontageInfo.AnimMontage != new_rep_montage_info_for_mesh.RepMontageInfo.AnimMontage ) || ( anim_montage_info.LocalMontageInfo.PlayBit != replicated_play_bit ) )
                {
                    anim_montage_info.LocalMontageInfo.PlayBit = replicated_play_bit;
                    PlayMontageSimulatedForMesh( new_rep_montage_info_for_mesh.Mesh, new_rep_montage_info_for_mesh.RepMontageInfo.AnimMontage, new_rep_montage_info_for_mesh.RepMontageInfo.PlayRate );
                }

                if ( anim_montage_info.LocalMontageInfo.AnimMontage == nullptr )
                {
                    ABILITY_LOG( Warning, TEXT( "OnRep_ReplicatedAnimMontage: PlayMontageSimulated failed. Name: %s, AnimMontage: %s" ), *GetNameSafe( this ), *GetNameSafe( new_rep_montage_info_for_mesh.RepMontageInfo.AnimMontage ) );
                    return;
                }

                // Play Rate has changed
                if ( anim_instance->Montage_GetPlayRate( anim_montage_info.LocalMontageInfo.AnimMontage ) != new_rep_montage_info_for_mesh.RepMontageInfo.PlayRate )
                {
                    anim_instance->Montage_SetPlayRate( anim_montage_info.LocalMontageInfo.AnimMontage, new_rep_montage_info_for_mesh.RepMontageInfo.PlayRate );
                }

                // Compressed Flags
                const auto is_stopped = anim_instance->Montage_GetIsStopped( anim_montage_info.LocalMontageInfo.AnimMontage );
                const auto replicated_is_stopped = static_cast< bool >( new_rep_montage_info_for_mesh.RepMontageInfo.IsStopped );

                // Process stopping first, so we don't change sections and cause blending to pop.
                if ( replicated_is_stopped )
                {
                    if ( !is_stopped )
                    {
                        CurrentMontageStopForMesh( new_rep_montage_info_for_mesh.Mesh, new_rep_montage_info_for_mesh.RepMontageInfo.BlendTime );
                    }
                }
                else if ( !new_rep_montage_info_for_mesh.RepMontageInfo.SkipPositionCorrection )
                {
                    const auto rep_section_id = anim_montage_info.LocalMontageInfo.AnimMontage->GetSectionIndexFromPosition( new_rep_montage_info_for_mesh.RepMontageInfo.Position );
                    const auto rep_next_section_id = static_cast< int32 >( new_rep_montage_info_for_mesh.RepMontageInfo.NextSectionID ) - 1;

                    // And NextSectionID for the replicated SectionID.
                    if ( rep_section_id != INDEX_NONE )
                    {
                        const auto next_section_id = anim_instance->Montage_GetNextSectionID( anim_montage_info.LocalMontageInfo.AnimMontage, rep_section_id );

                        // If NextSectionID is different than the replicated one, then set it.
                        if ( next_section_id != rep_next_section_id )
                        {
                            anim_instance->Montage_SetNextSection( anim_montage_info.LocalMontageInfo.AnimMontage->GetSectionName( rep_section_id ), anim_montage_info.LocalMontageInfo.AnimMontage->GetSectionName( rep_next_section_id ), anim_montage_info.LocalMontageInfo.AnimMontage );
                        }

                        // Make sure we haven't received that update too late and the client hasn't already jumped to another section.
                        const auto current_section_id = anim_montage_info.LocalMontageInfo.AnimMontage->GetSectionIndexFromPosition( anim_instance->Montage_GetPosition( anim_montage_info.LocalMontageInfo.AnimMontage ) );
                        if ( ( current_section_id != rep_section_id ) && ( current_section_id != rep_next_section_id ) )
                        {
                            // Client is in a wrong section, teleport him into the beginning of the right section
                            const auto section_start_time = anim_montage_info.LocalMontageInfo.AnimMontage->GetAnimCompositeSection( rep_section_id ).GetTime();
                            anim_instance->Montage_SetPosition( anim_montage_info.LocalMontageInfo.AnimMontage, section_start_time );
                        }
                    }

                    // Update Position. If error is too great, jump to replicated position.
                    const auto current_position = anim_instance->Montage_GetPosition( anim_montage_info.LocalMontageInfo.AnimMontage );
                    const auto current_section_id = anim_montage_info.LocalMontageInfo.AnimMontage->GetSectionIndexFromPosition( current_position );
                    const auto delta_position = new_rep_montage_info_for_mesh.RepMontageInfo.Position - current_position;

                    // Only check threshold if we are located in the same section. Different sections require a bit more work as we could be jumping around the timeline.
                    // And therefore DeltaPosition is not as trivial to determine.
                    if ( ( current_section_id == rep_section_id ) && ( FMath::Abs( delta_position ) > montage_rep_pos_err_threshold ) && ( new_rep_montage_info_for_mesh.RepMontageInfo.IsStopped == 0 ) )
                    {
                        // fast forward to server position and trigger notifies
                        if ( auto * montage_instance = anim_instance->GetActiveInstanceForMontage( new_rep_montage_info_for_mesh.RepMontageInfo.AnimMontage ) )
                        {
                            // Skip triggering notifies if we're going backwards in time, we've already triggered them.
                            const auto delta_time = !FMath::IsNearlyZero( new_rep_montage_info_for_mesh.RepMontageInfo.PlayRate )
                                                        ? ( delta_position / new_rep_montage_info_for_mesh.RepMontageInfo.PlayRate )
                                                        : 0.f;
                            if ( delta_time >= 0.f )
                            {
                                montage_instance->UpdateWeight( delta_time );
                                montage_instance->HandleEvents( current_position, new_rep_montage_info_for_mesh.RepMontageInfo.Position, nullptr );
                                anim_instance->TriggerAnimNotifies( delta_time );
                            }
                        }
                        anim_instance->Montage_SetPosition( anim_montage_info.LocalMontageInfo.AnimMontage, new_rep_montage_info_for_mesh.RepMontageInfo.Position );
                    }
                }
            }
        }
    }
}

bool UGASExtAbilitySystemComponent::IsReadyForReplicatedMontageForMesh() const
{
    /** Children may want to override this for additional checks (e.g, "has skin been applied") */
    return true;
}

void UGASExtAbilitySystemComponent::ServerCurrentMontageSetNextSectionNameForMesh_Implementation( USkeletalMeshComponent * mesh, UAnimMontage * client_anim_montage, const float client_position, const FName section_name, const FName next_section_name )
{
    auto * anim_instance = GetMeshAnimInstance( mesh );

    auto & anim_montage_info = GetLocalAnimMontageInfoForMesh( mesh );

    if ( anim_instance )
    {
        auto * current_anim_montage = anim_montage_info.LocalMontageInfo.AnimMontage;
        if ( client_anim_montage == current_anim_montage )
        {
            // Set NextSectionName
            anim_instance->Montage_SetNextSection( section_name, next_section_name, current_anim_montage );

            // Correct position if we are in an invalid section
            const auto current_position = anim_instance->Montage_GetPosition( current_anim_montage );
            const auto current_section_id = current_anim_montage->GetSectionIndexFromPosition( current_position );
            const auto current_section_name = current_anim_montage->GetSectionName( current_section_id );

            const auto client_section_id = current_anim_montage->GetSectionIndexFromPosition( client_position );
            const auto client_current_section_name = current_anim_montage->GetSectionName( client_section_id );
            if ( ( current_section_name != client_current_section_name ) || ( current_section_name != section_name ) )
            {
                // We are in an invalid section, jump to client's position.
                anim_instance->Montage_SetPosition( current_anim_montage, client_position );
            }

            // Update replicated version for Simulated Proxies if we are on the server.
            if ( IsOwnerActorAuthoritative() )
            {
                AnimMontage_UpdateReplicatedDataForMesh( mesh );
            }
        }
    }
}

bool UGASExtAbilitySystemComponent::ServerCurrentMontageSetNextSectionNameForMesh_Validate( USkeletalMeshComponent * mesh, UAnimMontage * client_anim_montage, const float client_position, const FName section_name, const FName next_section_name )
{
    return true;
}

void UGASExtAbilitySystemComponent::ServerCurrentMontageJumpToSectionNameForMesh_Implementation( USkeletalMeshComponent * mesh, UAnimMontage * client_anim_montage, const FName section_name )
{
    auto * anim_instance = GetMeshAnimInstance( mesh );
    auto & anim_montage_info = GetLocalAnimMontageInfoForMesh( mesh );

    if ( anim_instance )
    {
        auto * current_anim_montage = anim_montage_info.LocalMontageInfo.AnimMontage;
        if ( client_anim_montage == current_anim_montage )
        {
            // Set NextSectionName
            anim_instance->Montage_JumpToSection( section_name, current_anim_montage );

            // Update replicated version for Simulated Proxies if we are on the server.
            if ( IsOwnerActorAuthoritative() )
            {
                AnimMontage_UpdateReplicatedDataForMesh( mesh );
            }
        }
    }
}

bool UGASExtAbilitySystemComponent::ServerCurrentMontageJumpToSectionNameForMesh_Validate( USkeletalMeshComponent * mesh, UAnimMontage * client_anim_montage, const FName section_name )
{
    return true;
}

void UGASExtAbilitySystemComponent::ServerCurrentMontageSetPlayRateForMesh_Implementation( USkeletalMeshComponent * mesh, UAnimMontage * client_anim_montage, const float play_rate )
{
    auto * anim_instance = GetMeshAnimInstance( mesh );
    auto & anim_montage_info = GetLocalAnimMontageInfoForMesh( mesh );

    if ( anim_instance )
    {
        auto * current_anim_montage = anim_montage_info.LocalMontageInfo.AnimMontage;
        if ( client_anim_montage == current_anim_montage )
        {
            // Set PlayRate
            anim_instance->Montage_SetPlayRate( anim_montage_info.LocalMontageInfo.AnimMontage, play_rate );

            // Update replicated version for Simulated Proxies if we are on the server.
            if ( IsOwnerActorAuthoritative() )
            {
                AnimMontage_UpdateReplicatedDataForMesh( mesh );
            }
        }
    }
}

bool UGASExtAbilitySystemComponent::ServerCurrentMontageSetPlayRateForMesh_Validate( USkeletalMeshComponent * mesh, UAnimMontage * client_anim_montage, const float play_rate )
{
    return true;
}
