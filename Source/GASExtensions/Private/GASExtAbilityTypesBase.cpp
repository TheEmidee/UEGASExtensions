#include "GASExtAbilityTypesBase.h"

bool FGASExtGameplayEffectContext::NetSerialize( FArchive & ar, UPackageMap * map, bool & out_success )
{
    return FGameplayEffectContext::NetSerialize( ar, map, out_success );
}

bool FGASExtGameplayAbilityTargetData_LocationInfo::NetSerialize( FArchive & archive, UPackageMap * package_map, bool & success )
{
    return FGameplayAbilityTargetData_LocationInfo::NetSerialize( archive, package_map, success );
}
