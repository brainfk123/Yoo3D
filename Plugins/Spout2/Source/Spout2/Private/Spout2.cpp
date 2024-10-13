// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "Spout2.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FSpout2Module"

void FSpout2Module::StartupModule()
{
}

void FSpout2Module::ShutdownModule()
{
}

DEFINE_LOG_CATEGORY(LogSpout2);

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSpout2Module, Spout2)