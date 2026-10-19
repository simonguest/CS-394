#include "LlamaCppPlugin.h"

#define LOCTEXT_NAMESPACE "FLlamaCppPluginModule"

void FLlamaCppPluginModule::StartupModule()
{
    UE_LOG(LogTemp, Log, TEXT("LlamaCpp Plugin started"));
}

void FLlamaCppPluginModule::ShutdownModule()
{
    UE_LOG(LogTemp, Log, TEXT("LlamaCpp Plugin shutdown"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FLlamaCppPluginModule, LlamaCppPlugin)