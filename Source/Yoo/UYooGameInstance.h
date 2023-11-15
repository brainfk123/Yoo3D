// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "UYooGameInstance.generated.h"

/** 
* Tick function that calls UYooGameInstance::Tick
**/
USTRUCT()
struct FYooTickFunction : public FTickFunction
{
	GENERATED_USTRUCT_BODY()

	/**  the target of this tick **/
	UPROPERTY()
	class UYooGameInstance*	Target;

	/** 
		* Abstract function actually execute the tick. 
		* @param DeltaTime - frame time to advance, in seconds
		* @param TickType - kind of tick for this frame
		* @param CurrentThread - thread we are executing on, useful to pass along as new tasks are created
		* @param MyCompletionGraphEvent - completion event for this task. Useful for holding the completetion of this task until certain child tasks are complete.
	**/
	virtual void ExecuteTick(float DeltaTime, ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent) override;
	/** Abstract function to describe this tick. Used to print messages about illegal cycles in the dependency graph **/
	virtual FString DiagnosticMessage() override;
	virtual FName DiagnosticContext(bool bDetailed) override;
};

UCLASS()
class YOO_API UYooGameInstance : public UGameInstance
{
	GENERATED_BODY()

	virtual void OnStart() override;

	virtual void OnWorldChanged(UWorld* OldWorld, UWorld* NewWorld) override;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> WidgetClass;

	UPROPERTY()
	UUserWidget* UMGWidget;
	
	UFUNCTION(BlueprintCallable)
	void OpenAsWindow(FString WindowTitle, FVector2D WindowPosition, FVector2D WindowSize,  bool bUseOsBorder, bool bHasTitleBar, bool bDragEverywhere);
 
	UFUNCTION(BlueprintCallable)
	void CloseWindow();
	
	UFUNCTION(BlueprintCallable)
	void SetManualDpiScaling(bool bEnableManualScaling);
	
	UFUNCTION(BlueprintCallable)
	void OverrideManualDpiScalingWindowSize(FVector2D OverrideWindowSize);

	UPROPERTY(EditAnywhere)
	FString TitleName;
	
	UPROPERTY(EditDefaultsOnly, Category=Tick)
	struct FYooTickFunction PrimaryTick;
protected:
	TSharedPtr<SWindow> Window;
	FOnWindowClosed WindowClosedDelegate;
	FVector2D WindowOriginalSize;
	bool bUseManualDPIScaling;
	
	void OnWindowClose(const TSharedRef<SWindow>& InWindow);
public:
	// Called every frame
	virtual void Tick(float DeltaTime);
};

template<>
struct TStructOpsTypeTraits<FYooTickFunction> : public TStructOpsTypeTraitsBase2<FYooTickFunction>
{
	enum
	{
		WithCopy = false
	};
};