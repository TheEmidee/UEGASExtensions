#pragma once

#include "Components/GameFrameworkComponentManager.h"
#include "GFEGameFeatureAction_WorldActionBase.h"
#include "Abilities/GASExtAbilitySet.h"

#include <CoreMinimal.h>

#include "GASExtGameFeatureAction_AddAbilities.generated.h"

class UGASExtAbilitySet;
struct FGameFeatureDeactivatingContext;

// This comes from the sample ValleyOfTheAncient

USTRUCT( BlueprintType )
struct FGASExtGameFeatureAbilityMapping
{
    GENERATED_BODY()

    // Type of ability to grant
    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    TSoftClassPtr< UGameplayAbility > AbilityType;

    // Input action to bind the ability to, if any (can be left unset)
    // Will be uncommented when switching to UE5
    /*UPROPERTY( EditAnywhere, BlueprintReadOnly )
    TSoftClassPtr< UInputAction > AbilityType;*/
};

USTRUCT( BlueprintType )
struct FGASExtGameFeatureAttributesMapping
{
    GENERATED_BODY()

    // Ability set to grant
    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    TSoftClassPtr< UAttributeSet > AttributeSetType;

    // Data table referent to initialize the attributes with, if any (can be left unset)
    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    TSoftObjectPtr< UDataTable > InitializationData;
};

USTRUCT()
struct FGASExtGameFeatureAbilitiesEntry
{
    GENERATED_BODY()

    // The base actor class to add to
    UPROPERTY( EditAnywhere, Category = "Abilities" )
    TSoftClassPtr< AActor > ActorClass;

    // List of abilities to grant to actors of the specified class
    UPROPERTY( EditAnywhere, Category = "Abilities" )
    TArray< FGASExtGameFeatureAbilityMapping > GrantedAbilities;

    UPROPERTY( EditAnywhere, Category = "Effects" )
    TArray< TSoftClassPtr< UGameplayEffect > > GrantedEffects;

    // List of attribute sets to grant to actors of the specified class
    UPROPERTY( EditAnywhere, Category = "Attributes" )
    TArray< FGASExtGameFeatureAttributesMapping > GrantedAttributes;

    UPROPERTY( EditAnywhere, Category = "Attributes", meta = ( AssetBundles = "Client,Server" ) )
    TArray< TSoftObjectPtr< const UGASExtAbilitySet > > GrantedAbilitySets;

    UPROPERTY( EditAnywhere, Category = "Tags" )
    FGameplayTagContainer LooseGameplayTags;
};

UCLASS( meta = ( DisplayName = "Add Abilities" ) )
class GASEXTENSIONS_API UGASExtGameFeatureAction_AddAbilities final : public UGFEGameFeatureAction_WorldActionBase
{
    GENERATED_BODY()

public:
    void OnGameFeatureActivating() override;
    void OnGameFeatureDeactivating( FGameFeatureDeactivatingContext & context ) override;

#if WITH_EDITORONLY_DATA
    void AddAdditionalAssetBundleData( FAssetBundleData & asset_bundle_data ) override;
#endif

#if WITH_EDITOR
    EDataValidationResult IsDataValid( TArray< FText > & validation_errors ) override;
#endif

    static const FName NAME_AbilityReady;

private:
    void AddToWorld( const FWorldContext & world_context ) override;

    void Reset();
    void HandleActorExtension( AActor * actor, FName event_name, int32 entry_index );
    void AddActorAbilities( AActor * actor, const FGASExtGameFeatureAbilitiesEntry & abilities_entry );
    void RemoveActorAbilities( AActor * actor );

    template < class ComponentType >
    ComponentType * FindOrAddComponentForActor( AActor * actor, const FGASExtGameFeatureAbilitiesEntry & abilities_entry )
    {
        return Cast< ComponentType >( FindOrAddComponentForActor( ComponentType::StaticClass(), actor, abilities_entry ) );
    }

    UActorComponent * FindOrAddComponentForActor( UClass * component_type, AActor * actor, const FGASExtGameFeatureAbilitiesEntry & abilities_entry );

    UPROPERTY( EditAnywhere, Category = "Abilities", meta = ( TitleProperty = "ActorClass", ShowOnlyInnerProperties ) )
    TArray< FGASExtGameFeatureAbilitiesEntry > AbilitiesList;

    struct FActorExtensions
    {
        TArray< FGameplayAbilitySpecHandle > Abilities;
        TArray< UAttributeSet * > Attributes;
        TArray< FActiveGameplayEffectHandle > Effects;
        TArray< FGASExtAbilitySet_GrantedHandles > AbilitySetHandles;
        FGameplayTagContainer Tags;
    };
    TMap< AActor *, FActorExtensions > ActiveExtensions;
    TArray< TSharedPtr< FComponentRequestHandle > > ComponentRequests;
};
