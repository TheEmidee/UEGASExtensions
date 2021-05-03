#include "GASExtensionsModule.h"

class FGASExtensionsModule final : public IGASExtensionsModule
{
public:
    void StartupModule() override;
    void ShutdownModule() override;
};

IMPLEMENT_MODULE( FGASExtensionsModule, GASExtensions )

void FGASExtensionsModule::StartupModule()
{
}

void FGASExtensionsModule::ShutdownModule()
{
}