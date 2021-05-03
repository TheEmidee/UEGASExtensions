#include "GASExtAbilityTypesBase.h"

bool FGASExtGameplayAbilityTargetData_LocationInfo::NetSerialize( FArchive & archive, UPackageMap * package_map, bool & success )
{
    return FGameplayAbilityTargetData_LocationInfo::NetSerialize( archive, package_map, success );
}