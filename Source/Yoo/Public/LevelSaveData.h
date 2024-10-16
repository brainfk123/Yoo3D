#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "LevelSaveData.generated.h"

class ASpoutCamera;
class AEditableObject;

UCLASS(BlueprintType)
class YOO_API ULevelSaveData : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY()
	FTransform ViewTransform;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FName LevelName;

	UPROPERTY()
	TObjectPtr<ULevel> Level;

	UPROPERTY()
	TObjectPtr<UWorld> DummyWorld;

	UPROPERTY()
	TObjectPtr<ASpoutCamera> MainCamera;
};
