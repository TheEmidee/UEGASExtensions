#pragma once

#include <CoreMinimal.h>
#include <Modules/ModuleInterface.h>
#include <Modules/ModuleManager.h>
#include <Stats/Stats.h>

class GASEXTENSIONS_API IGASExtensionsModule : public IModuleInterface
{

public:
    static IGASExtensionsModule & Get()
    {
        static auto & singleton = FModuleManager::LoadModuleChecked< IGASExtensionsModule >( "GASExtensions" );
        return singleton;
    }

    static bool IsAvailable()
    {
        return FModuleManager::Get().IsModuleLoaded( "GASExtensions" );
    }
};
