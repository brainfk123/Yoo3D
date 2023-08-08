// Copyright Epic Games, Inc. All Rights Reserved.

#include "YooGameMode.h"
#include "YooCharacter.h"
#include "UObject/ConstructorHelpers.h"

AYooGameMode::AYooGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
