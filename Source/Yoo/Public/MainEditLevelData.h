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

	UPROPERTY(BlueprintReadOnly, interp, Category="Character|Lighting", meta=(DisplayName = "Intensity", ClampMin = "0.0", UIMax = "20.0"))
	float Intensity = 2.0;

	UPROPERTY(BlueprintReadOnly, interp, Category="Character|Lighting", meta=(HideAlphaChannel))
	FColor LightColor = FColor::White;
};

UCLASS()
class YOO_API UMainEditLevelData : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FMainEditLevelInfo> LevelData;
};
