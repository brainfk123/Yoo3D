// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UMGWindow.generated.h"

UCLASS(Blueprintable)
class YOO_API AUMGWindow : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AUMGWindow();
	
	UFUNCTION(BlueprintCallable)
	void OpenAsWindow(UUserWidget* Widget, FString WindowTitle, FVector2D WindowPosition, FVector2D WindowSize,  bool bUseOsBorder, bool bHasTitleBar, bool bDragEverywhere);
 
	UFUNCTION(BlueprintCallable)
	void CloseWindow();
	
	UFUNCTION(BlueprintCallable)
	void SetManualDpiScaling(bool bEnableManualScaling);
	
	UFUNCTION(BlueprintCallable)
	void OverrideManualDpiScalingWindowSize(FVector2D OverrideWindowSize);
	
	UPROPERTY(EditAnywhere)
	UUserWidget* UmgWidget;

	UPROPERTY(EditAnywhere)
	bool bShowOnStartup;

	UPROPERTY(EditAnywhere)
	FString TitleName;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	TSharedPtr<SWindow> Window;
	FOnWindowClosed WindowClosedDelegate;
	FVector2D WindowOriginalSize;
	bool bUseManualDPIScaling;
	
	void OnWindowClose(const TSharedRef<SWindow>& InWindow);
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
