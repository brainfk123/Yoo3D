// Fill out your copyright notice in the Description page of Project Settings.
#include "UYooGameInstance.h"
#include "Blueprint/UserWidget.h"
#include "Engine/UserInterfaceSettings.h"
#include "Kismet/GameplayStatics.h"

void UYooGameInstance::OnStart()
{
	Super::OnStart();
	
	PrimaryTick.Target = this;
	PrimaryTick.bCanEverTick = true;
	
	if (WidgetClass)
	{
		UMGWidget = NewObject<UUserWidget>(this, WidgetClass);
		OpenAsWindow(TitleName, FVector2d::ZeroVector, {720, 1280}, false, true, true);
	}
}

void UYooGameInstance::OnWorldChanged(UWorld* OldWorld, UWorld* NewWorld)
{
	Super::OnWorldChanged(OldWorld, NewWorld);

	PrimaryTick.UnRegisterTickFunction();
	if (NewWorld)
	{
		PrimaryTick.RegisterTickFunction(NewWorld->PersistentLevel);
	}
	PrimaryTick.SetTickFunctionEnable(true);
}

void UYooGameInstance::OpenAsWindow(FString WindowTitle, FVector2D WindowPosition, FVector2D WindowSize,
                                    bool bUseOsBorder, bool bHasTitleBar, bool bDragEverywhere)
{
	// Remember original window size for the manual DPI scaling
	WindowOriginalSize = WindowSize;
	
	/*
	 * SET UP THE NEW WINDOW
	 */
 
	// Create Slate Window
	Window = SNew(SWindow)
	/* set all properties we got passed with the call */
		.Title(FText::FromString(WindowTitle))
		.ScreenPosition(WindowPosition)
		.ClientSize(WindowSize)
		.UseOSWindowBorder(bUseOsBorder)
		.bDragAnywhere(bDragEverywhere)
		.CreateTitleBar(bHasTitleBar)
	/* set a few more interesting ones, just to show they exist */
		.AutoCenter(EAutoCenter::PrimaryWorkArea)
		.SaneWindowPlacement(true)
		.SizingRule(ESizingRule::UserSized)
		.Type(EWindowType::Normal)
		.InitialOpacity(1.0f)
		.HasCloseButton(true)
		.MaxHeight(9999)
		.MaxWidth(9999)
		.SupportsTransparency(EWindowTransparency::None)
		.LayoutBorder(FMargin {6, 6})
		.UserResizeBorder(FMargin {4, 4})
		.AdjustInitialSizeAndPositionForDPIScale(false)
		.MinHeight(640)
		.MinWidth(360)
	;
 
	// Add our new window to the Slate subsystem (which essentially opens it)
	FSlateApplication::Get().AddWindow(Window.ToSharedRef());
 
	// Make sure our new window doesn't hide behind some others
	Window.Get()->BringToFront(true);
 
	// Bind a method to the OnClosed event, to clean up should the user, well, close the window
	WindowClosedDelegate.BindUObject(this, &UYooGameInstance::OnWindowClose);
	Window->SetOnWindowClosed(WindowClosedDelegate);
 
	/*
	 * SET UP THE UMG USER WIDGET AS WINDOW CONTENT
	 */
 
	// Finally fetch the Slate widget from the UMG one....
	TSharedRef<SWidget> SlateWidget = UMGWidget->TakeWidget();
 
	// ....and set it as content for our newly created/opened window
	Window->SetContent(SlateWidget);
}

void UYooGameInstance::CloseWindow()
{
	Window->RequestDestroyWindow();
}

void UYooGameInstance::SetManualDpiScaling(bool bEnableManualScaling)
{
	bUseManualDPIScaling = bEnableManualScaling;
}

void UYooGameInstance::OverrideManualDpiScalingWindowSize(FVector2D OverrideWindowSize)
{
	WindowOriginalSize = OverrideWindowSize;
}

void UYooGameInstance::OnWindowClose(const TSharedRef<SWindow>& InWindow)
{
	// Clean up the UMG widget, i.e. remove it from the window and invalidate our reference
	UMGWidget->RemoveFromParent();
	UMGWidget = nullptr;
 
	// Also clean up the window reference
	Window.Reset();
	Window = nullptr;

	APlayerController* TargetPC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if( TargetPC )
	{
		TargetPC->ConsoleCommand("quit");
	}
}

// Called every frame
void UYooGameInstance::Tick(float DeltaTime)
{
	FGeometry WindowGeometry;
	FVector2D Scale;
	float DPIFactor;

	/*
	 * Apply DPI scaling to the window, so the UI contents scale proportionally with the window size.
	 * This might be a bad spot to do this, this would be better placed in some OnResized event,
	 * but hey, this is only an example ;-)
	 */
	if (Window)
	{
		// Fetch the current window geometry (size, position, etc.)
		WindowGeometry = Window->GetWindowGeometryInScreen();

		if (bUseManualDPIScaling)
		{
			// calculate the DPI factor the window contents manually, sometimes this gives a nicer UI scaling
			Scale.X = WindowGeometry.GetAbsoluteSize().X / WindowOriginalSize.X;
			Scale.Y = WindowGeometry.GetAbsoluteSize().Y / WindowOriginalSize.Y;
			DPIFactor = Scale.X; // or .Y, whatever you want the dominant edge to be
		}
		else
		{
			// fetch the DPI scale factor for the window contents from the projects configured DPI curve
			DPIFactor = GetDefault<UUserInterfaceSettings>(UUserInterfaceSettings::StaticClass())
				->GetDPIScaleBasedOnSize(FIntPoint(WindowGeometry.GetAbsoluteSize().X,
				                                   WindowGeometry.GetAbsoluteSize().Y));
		}

		// Set the new DPI scale factor for the window
		Window->SetDPIScaleFactor(DPIFactor);
	}
}

void FYooTickFunction::ExecuteTick(float DeltaTime, enum ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
{
	if (Target && IsValidChecked(Target) && !Target->IsUnreachable())
	{
		if (TickType != LEVELTICK_ViewportsOnly)
		{
			FScopeCycleCounterUObject ActorScope(Target);
			Target->Tick(DeltaTime);
		}
	}
}

FString FYooTickFunction::DiagnosticMessage()
{
	return Target->GetFullName() + TEXT("[Tick]");
}

FName FYooTickFunction::DiagnosticContext(bool bDetailed)
{
	if (bDetailed)
	{
		// Format is "ActorNativeClass/ActorClass"
		FString ContextString = FString::Printf(TEXT("%s/%s"), *GetParentNativeClass(Target->GetClass())->GetName(), *Target->GetClass()->GetName());
		return FName(*ContextString);
	}
	else
	{
		return GetParentNativeClass(Target->GetClass())->GetFName();
	}
}