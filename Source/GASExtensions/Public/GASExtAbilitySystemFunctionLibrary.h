#pragma once

#include "GASExtAbilityTypesBase.h"

#include <CoreMinimal.h>
#include <GameplayEffectTypes.h>
#include <Kismet/BlueprintFunctionLibrary.h>

#include "GASExtAbilitySystemFunctionLibrary.generated.h"

class UGameplayAbility;
struct FGameplayEventData;
class UGameplayEffect;

UCLASS()
class GASEXTENSIONS_API UGASExtAbilitySystemFunctionLibrary final : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable, Category = "Ability|GameplayEffects" )
    static FGASExtGameplayEffectContainerSpec MakeEffectContainerSpecFromEffectContainer( const UGameplayAbility * ability, const FGASExtGameplayEffectContainer & effect_container, const FGameplayEventData & event_data );

    UFUNCTION( BlueprintCallable, Category = "Ability|GameplayEffects" )
    static FGASExtGameplayEffectContainerSpec MakeEffectContainerSpecFromEffectContainerAndHitResult( const UGameplayAbility * ability, const FGASExtGameplayEffectContainer & effect_container, const FGameplayEventData & event_data, FHitResult hit_result );

    UFUNCTION( BlueprintCallable, Category = "Ability|GameplayEffects" )
    static TArray< FActiveGameplayEffectHandle > ApplyGameplayEffectContainerSpec( const FGASExtGameplayEffectContainerSpec & effect_container_spec );

    UFUNCTION( BlueprintCallable, Category = "Ability|GameplayEffects" )
    static FGameplayEffectSpecHandle MakeGameplayEffectSpecHandle( TSubclassOf< UGameplayEffect > effect_class, AActor * instigator, AActor * effect_causer, const UGameplayAbility * ability = nullptr );

    UFUNCTION( BlueprintPure, Category = "Ability|AttributeSet" )
    static float GetScalableFloatValue( const FScalableFloat & scalable_float );

    UFUNCTION( BlueprintPure, Category = "Ability|GameplayEffects" )
    static bool IsGameplayEffectHandleValid( FActiveGameplayEffectHandle gameplay_effect_handle );
};
