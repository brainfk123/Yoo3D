// Fill out your copyright notice in the Description page of Project Settings.


#include "UMGWindow.h"
#include "GenericPlatform/GenericWindowDefinition.h"
#include "Blueprint/UserWidget.h"
#include "Engine/UserInterfaceSettings.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
AUMGWindow::AUMGWindow()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Window = nullptr;
	bUseManualDPIScaling = false;
}

void AUMGWindow::OpenAsWindow(UUserWidget* Widget, FString WindowTitle, FVector2D WindowPosition, FVector2D WindowSize,
	bool bUseOsBorder, bool bHasTitleBar, bool bDragEverywhere)
{
	// Remember the reference to the UMG user widget, for later use
	UmgWidget = Widget;
 
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
	WindowClosedDelegate.BindUObject(this, &AUMGWindow::OnWindowClose);
	Window->SetOnWindowClosed(WindowClosedDelegate);
 
	/*
	 * SET UP THE UMG USER WIDGET AS WINDOW CONTENT
	 */
 
	// Finally fetch the Slate widget from the UMG one....
	TSharedRef<SWidget> SlateWidget = Widget->TakeWidget();
 
	// ....and set it as content for our newly created/opened window
	Window->SetContent(SlateWidget);
}

void AUMGWindow::CloseWindow()
{
	Window->RequestDestroyWindow();
}

void AUMGWindow::SetManualDpiScaling(bool bEnableManualScaling)
{
	bUseManualDPIScaling = bEnableManualScaling;
}

void AUMGWindow::OverrideManualDpiScalingWindowSize(FVector2D OverrideWindowSize)
{
	WindowOriginalSize = OverrideWindowSize;
}

void AUMGWindow::OnWindowClose(const TSharedRef<SWindow>& InWindow)
{
	// Clean up the UMG widget, i.e. remove it from the window and invalidate our reference
	UmgWidget->RemoveFromParent();
	UmgWidget = nullptr;
 
	// Also clean up the window reference
	Window.Reset();
	Window = nullptr;

	APlayerController* TargetPC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if( TargetPC )
	{
		TargetPC->ConsoleCommand("quit");
	}
}

// Called when the game starts or when spawned
void AUMGWindow::BeginPlay()
{
	Super::BeginPlay();
	if (bShowOnStartup && IsValid(UmgWidget))
	{
		OpenAsWindow(UmgWidget, TitleName, FVector2d::ZeroVector, {720, 1280}, false, true, true);
	}
}

// Called every frame
void AUMGWindow::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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

