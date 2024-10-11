#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MainEditLevelData.generated.h"

USTRUCT(BlueprintType)
struct FMainEditLevelInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName LevelName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText LevelDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UTexture2D> LevelThumbnail;
};

UCLASS()
class YOO_API UMainEditLevelData : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FMainEditLevelInfo> LevelData;
};
