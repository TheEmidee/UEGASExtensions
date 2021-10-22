#include "GASExtAbilityTypesBase.h"

bool FGASExtGameplayEffectContext::NetSerialize( FArchive & ar, UPackageMap * map, bool & out_success )
{
    uint32 RepBits = 0;
    if ( ar.IsSaving() )
    {
        if ( Instigator.IsValid() )
        {
            RepBits |= 1 << 0;
        }
        if ( EffectCauser.IsValid() )
        {
            RepBits |= 1 << 1;
        }
        if ( AbilityCDO.IsValid() )
        {
            RepBits |= 1 << 2;
        }
        if ( bReplicateSourceObject && SourceObject.IsValid() )
        {
            RepBits |= 1 << 3;
        }
        if ( Actors.Num() > 0 )
        {
            RepBits |= 1 << 4;
        }
        if ( HitResult.IsValid() )
        {
            RepBits |= 1 << 5;
        }
        if ( bHasWorldOrigin )
        {
            RepBits |= 1 << 6;
        }
        if ( IsValid( FallOffTypeClass ) )
        {
            RepBits |= 1 << 7;
        }
    }

    ar.SerializeBits( &RepBits, 10 );

    if ( RepBits & ( 1 << 0 ) )
    {
        ar << Instigator;
    }
    if ( RepBits & ( 1 << 1 ) )
    {
        ar << EffectCauser;
    }
    if ( RepBits & ( 1 << 2 ) )
    {
        ar << AbilityCDO;
    }
    if ( RepBits & ( 1 << 3 ) )
    {
        ar << SourceObject;
    }
    if ( RepBits & ( 1 << 4 ) )
    {
        SafeNetSerializeTArray_Default< 31 >( ar, Actors );
    }
    if ( RepBits & ( 1 << 5 ) )
    {
        if ( ar.IsLoading() )
        {
            if ( !HitResult.IsValid() )
            {
                HitResult = MakeShared< FHitResult >();
            }
        }
        HitResult->NetSerialize( ar, map, out_success );
    }
    if ( RepBits & ( 1 << 6 ) )
    {
        ar << WorldOrigin;
        bHasWorldOrigin = true;
    }
    else
    {
        bHasWorldOrigin = false;
    }

    if ( RepBits & ( 1 << 7 ) )
    {
        ar << FallOffTypeClass;
    }

    if ( ar.IsLoading() )
    {
        AddInstigator( Instigator.Get(), EffectCauser.Get() ); // Just to initialize InstigatorAbilitySystemComponent
    }

    out_success = true;
    return true;
}

bool FGASExtGameplayAbilityTargetData_LocationInfo::NetSerialize( FArchive & archive, UPackageMap * package_map, bool & success )
{
    return FGameplayAbilityTargetData_LocationInfo::NetSerialize( archive, package_map, success );
}
