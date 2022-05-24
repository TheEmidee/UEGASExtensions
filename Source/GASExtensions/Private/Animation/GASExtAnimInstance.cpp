#include "Animation/GASExtAnimInstance.h"

#include <AbilitySystemGlobals.h>

void UGASExtAnimInstance::InitializeWithAbilitySystem( UAbilitySystemComponent * asc )
{
    check( asc );

    GameplayTagPropertyMap.Initialize( this, asc );
}

#if WITH_EDITOR
EDataValidationResult UGASExtAnimInstance::IsDataValid( TArray< FText > & ValidationErrors )
{
    Super::IsDataValid( ValidationErrors );

    GameplayTagPropertyMap.IsDataValid( this, ValidationErrors );

    return ( ( ValidationErrors.Num() > 0 ) ? EDataValidationResult::Invalid : EDataValidationResult::Valid );
}
#endif // WITH_EDITOR

void UGASExtAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    if ( const auto * owning_actor = GetOwningActor() )
    {
        if ( auto * asc = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor( owning_actor ) )
        {
            InitializeWithAbilitySystem( asc );
        }
    }
}