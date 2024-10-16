#include "MainEditController.h"
#include "EditableObject.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "LevelSaveData.h"
#include "MainEditView.h"
#include "SpoutCamera.h"
#include "Yoo.h"
#include "YooGameMode.h"
#include "Blueprint/UserWidget.h"

AMainEditController::AMainEditController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
	CursorHit.Distance = 1.0;
	bIsInPlacementMode = false;
	bIsInInspectorMode = false;
}

void AMainEditController::SelectObject_Implementation(AEditableObject* Object)
{
	SelectedObject = Object;
	AttachGizmoTo(SelectedObject);
	OnSelectObject(SelectedObject);
	OnSelectObjectEvent.Broadcast(SelectedObject);
}

void AMainEditController::ReleaseSelection()
{
	SelectedObject = nullptr;
	AttachGizmoTo(SelectedObject);
	OnSelectObject(SelectedObject);
	OnSelectObjectEvent.Broadcast(SelectedObject);
}

void AMainEditController::BeginPlaceNewObject(
	TSubclassOf<AEditableObject> InClass,
	FOnPlacementFinished OnPlacementFinished)
{
	if (!GetWorld()->GetAuthGameMode<AYooGameMode>()->GetCurrentLevel())
	{
		OnPlacementFinished.ExecuteIfBound(nullptr);
		return;
	}
	
	if (InClass)
	{
		UStaticMesh* ProxyMesh = InClass.GetDefaultObject()->ProxyMesh;
		UTexture2D* ProxySprite = InClass.GetDefaultObject()->ProxySprite;
		if (ProxyMesh != nullptr)
		{
			MainEditView->PlacementProxyComponent->SetStaticMesh(ProxyMesh);
			MainEditView->PlacementProxyComponent->SetVisibility(true);
			MainEditView->PlacementProxyComponent->bRenderCustomDepth = InClass.GetDefaultObject()->bProxyMeshToonShade;
		}
		else if (ProxySprite != nullptr)
		{
			MainEditView->PlacementSpriteComponent->SetSprite(ProxySprite);
			MainEditView->PlacementSpriteComponent->SetVisibility(true);
		}
		
		MainEditView->PlacementProxyComponent->SetWorldLocation(GetPlacementHitLocation());
		bIsInPlacementMode = true;
		CachedPlacementClass = InClass;
		CachedOnPlacementFinished = OnPlacementFinished;
	}
}

AEditableObject* AMainEditController::GetSelectedObject()
{
	return SelectedObject;
}

void AMainEditController::SetSelectedActorTransform(const FTransform& NewTransform)
{
	if (SelectedObject)
	{
		SelectedObject->SetActorTransform(NewTransform);
		UpdateGizmo();
	}
}

void AMainEditController::BeginInspectCamera(ASpoutCamera* Camera)
{
	if (!bIsInPlacementMode)
	{
		bIsInInspectorMode = true;
		
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(DefaultMappingContext);
			Subsystem->AddMappingContext(InspectorMappingContext, 0);
		}
		
		if (MainHUDWidget)
		{
			MainHUDWidget->RemoveFromParent();
		}
		
		if (CameraInspectorWidget)
		{
			CameraInspectorWidget->AddToViewport();

			FInputModeGameOnly InputMode;
			InputMode.SetConsumeCaptureMouseDown(true);
			SetInputMode(InputMode);
			SetShowMouseCursor(false);
			
			SetInspectCameraToWidget(CameraInspectorWidget, Camera);
		}

		Camera->AddInspector();
		InspectorCamera = Camera;
	}
}

void AMainEditController::EndInspectCamera()
{
	if (bIsInInspectorMode)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(InspectorMappingContext);
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
		
		if (CameraInspectorWidget)
		{
			CameraInspectorWidget->RemoveFromParent();
			SetInspectCameraToWidget(CameraInspectorWidget, nullptr);
		}
		
		if (MainHUDWidget)
		{
			MainHUDWidget->AddToViewport();

			FInputModeGameAndUI InputMode;
			InputMode.SetHideCursorDuringCapture(false);
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			InputMode.SetWidgetToFocus(MainHUDWidget->TakeWidget());
			SetInputMode(InputMode);
			SetShowMouseCursor(true);
		}

		if (MainEditView)
		{
			MainEditView->SetActorTransform(InspectorCamera->GetActorTransform());
			MainEditView->AddActorWorldOffset(MainEditView->GetActorForwardVector() * -500.0);
		}

		InspectorCamera->StopMovement();
		UpdateGizmo();

		InspectorCamera->RemoveInspector();
		bIsInInspectorMode = false;
		InspectorCamera = nullptr;
	}
}

void AMainEditController::SetMainCamera(ASpoutCamera* InCamera)
{
	if (MainEditView)
	{
		MainEditView->MainCamera = InCamera;
	}
}

ASpoutCamera* AMainEditController::GetMainCamera()
{
	return MainEditView ? MainEditView->MainCamera : nullptr;
}

void AMainEditController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(RotateAction, ETriggerEvent::Triggered, this, &AMainEditController::OnRotate);
		EnhancedInputComponent->BindAction(PanAction, ETriggerEvent::Triggered, this, &AMainEditController::OnPan);
		EnhancedInputComponent->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &AMainEditController::OnZoom);
		EnhancedInputComponent->BindAction(LBPressedAction, ETriggerEvent::Triggered, this, &AMainEditController::OnLBPressed);
		EnhancedInputComponent->BindAction(RBPressedAction, ETriggerEvent::Triggered, this, &AMainEditController::OnRBPressed);
		EnhancedInputComponent->BindAction(LBReleasedAction, ETriggerEvent::Triggered, this, &AMainEditController::OnLBReleased);
		EnhancedInputComponent->BindAction(RBReleasedAction, ETriggerEvent::Triggered, this, &AMainEditController::OnRBReleased);
		EnhancedInputComponent->BindAction(LBClickAction, ETriggerEvent::Triggered, this, &AMainEditController::OnLBClicked);
		EnhancedInputComponent->BindAction(GizmoMoveAction, ETriggerEvent::Triggered, this, &AMainEditController::OnGizmoMove);
		EnhancedInputComponent->BindAction(GizmoRotateAction, ETriggerEvent::Triggered, this, &AMainEditController::OnGizmoRotate);
		EnhancedInputComponent->BindAction(GizmoScaleAction, ETriggerEvent::Triggered, this, &AMainEditController::OnGizmoScale);
		EnhancedInputComponent->BindAction(ConfirmAction, ETriggerEvent::Triggered, this, &AMainEditController::OnConfirm);
		EnhancedInputComponent->BindAction(CancelAction, ETriggerEvent::Triggered, this, &AMainEditController::OnCancel);
		EnhancedInputComponent->BindAction(DeleteAction, ETriggerEvent::Triggered, this, &AMainEditController::OnDelete);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMainEditController::OnMove);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMainEditController::OnLook);
	}
	else
	{
		UE_LOG(LogYoo, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AMainEditController::BeginPlay()
{
	Super::BeginPlay();

	//Add Input Mapping Context
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}
	
	if (GizmoClass)
	{
		FActorSpawnParameters Params;
		Params.Owner = this;
		Gizmo = GetWorld()->SpawnActor(GizmoClass, nullptr, nullptr, Params);
	}

	if (MainHUDClass)
	{
		MainHUDWidget = CreateWidget(this, MainHUDClass);
		if (MainHUDWidget)
		{
			MainHUDWidget->AddToViewport();

			FInputModeGameAndUI InputMode;
			InputMode.SetHideCursorDuringCapture(false);
			InputMode.SetWidgetToFocus(MainHUDWidget->TakeWidget());
			SetInputMode(InputMode);
		}
	}

	if (CameraInspectorClass)
	{
		CameraInspectorWidget = CreateWidget(this, CameraInspectorClass);
	}
}

void AMainEditController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	if (Gizmo)
	{
		Gizmo->Destroy();
	}
	Gizmo = nullptr;
}

void AMainEditController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bIsInPlacementMode)
	{
		MainEditView->PlacementProxyComponent->SetWorldLocation(GetPlacementHitLocation());
	}

	if (MainEditView)
	{
		MainEditView->UpdateCharacterLight();
	}

	UpdateInspectorRotation();
}

void AMainEditController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	MainEditView = Cast<AMainEditView>(InPawn);
}

void AMainEditController::OnUnPossess()
{
	if (auto Game = GetWorld()->GetAuthGameMode<AYooGameMode>())
	{
		SaveToLevelData(Game->GetCurrentLevel());
	}
	MainEditView = nullptr;
	
	Super::OnUnPossess();
}

float AMainEditController::ScreenDistToWorldDistScale() const
{
	FVector AOrigin;
	FVector ADir;
	FVector BOrigin;
	FVector BDir;
	DeprojectScreenPositionToWorld(0, 0, AOrigin, ADir);
	DeprojectScreenPositionToWorld(100.0, 0, BOrigin, BDir);
	return (BOrigin - AOrigin).Length() / 100.0;
}

FVector AMainEditController::GetPlacementHitLocation() const
{
	FVector HitLocation;
	FHitResult HitResult;
	if (GetHitResultUnderCursor(ECC_Visibility, true, HitResult))
	{
		HitLocation = HitResult.Location;
	}
	else
	{
		FVector Origin;
		FVector Direction;
		DeprojectMousePositionToWorld(Origin, Direction);
		HitLocation = Origin + Direction * 500.0;
	}
	return HitLocation;
}

void AMainEditController::UpdateInspectorRotation()
{
	if (bIsInInspectorMode && InspectorCamera)
	{
		FRotator Rotation = InspectorCamera->GetActorRotation();
		Rotation += RotationInput;

		Rotation.Pitch = FMath::ClampAngle(Rotation.Pitch, -89.0f, 89.0f);
		Rotation.Pitch = FRotator::ClampAxis(Rotation.Pitch);
		Rotation.Yaw = FRotator::ClampAxis(Rotation.Yaw);
		Rotation.Roll = FRotator::ClampAxis(Rotation.Roll);
		Rotation.Normalize();

		InspectorCamera->SetActorRotation(Rotation);
		RotationInput = FRotator::ZeroRotator;
	}
}

void AMainEditController::LoadFromLevelData(ULevelSaveData* NewLevel) const
{
	MainEditView->SetCameraTransform(NewLevel->ViewTransform);
}

void AMainEditController::SaveToLevelData(ULevelSaveData* NewLevel) const
{
	NewLevel->ViewTransform = MainEditView->GetCameraTransform();
	NewLevel->MainCamera = MainEditView->MainCamera;
}

void AMainEditController::SetCharacterLight(float Intensity, FColor LightColor) const
{
	if (MainEditView)
	{
		MainEditView->CharacterLight->SetIntensity(Intensity);
		MainEditView->CharacterLight->SetLightFColor(LightColor);
	}
}

void AMainEditController::OnLBPressed()
{
	UE_LOG(LogYoo, Verbose, TEXT("OnLBPressed"));
	FHitResult Hit;
	bCursorHit = GetHitResultUnderCursor(ECC_Visibility, true, Hit);
	if (bCursorHit)
	{
		CursorHit = Hit;
	}
	bSelectionHit = GetHitResultUnderCursor(ECC_GameTraceChannel1, true, Hit);
	if (bSelectionHit)
	{
		SelectionHit = Hit;
	}
}

void AMainEditController::OnRBPressed()
{
	UE_LOG(LogYoo, Verbose, TEXT("OnRBPressed"));
	FHitResult Hit;
	bCursorHit = GetHitResultUnderCursor(ECC_Visibility, true, Hit);
	if (bCursorHit)
	{
		CursorHit = Hit;
	}
}

void AMainEditController::OnLBReleased()
{
	UE_LOG(LogYoo, Verbose, TEXT("OnLBReleased"));
}

void AMainEditController::OnRBReleased()
{
	UE_LOG(LogYoo, Verbose, TEXT("OnRBReleased"));
}

void AMainEditController::OnRotate(const FInputActionValue& Value)
{
	UE_LOG(LogYoo, Verbose, TEXT("OnRotate %s"), *Value.ToString());
	if (MainEditView)
	{
		float Distance = FMath::Clamp(CursorHit.Distance, 1.0, 10000.0);
		FVector TargetCenter = CursorHit.TraceStart + Distance * (CursorHit.TraceEnd - CursorHit.TraceStart).GetSafeNormal();
		FRotator BaseRotate = FRotator(Value.Get<FVector2D>().X, Value.Get<FVector2D>().Y, 0.0);
		MainEditView->RotateCamera(TargetCenter, BaseRotate);
	}
}

void AMainEditController::OnPan(const FInputActionValue& Value)
{
	UE_LOG(LogYoo, Verbose, TEXT("OnPan %s"), *Value.ToString());
	if (MainEditView)
	{
		const float TargetDistance = (CursorHit.Location - MainEditView->GetLocation()) | MainEditView->GetForward();
		const float Distance = FMath::Clamp(TargetDistance, 1.0, 10000.0);
		FVector Origin;
		FVector Direction;
		DeprojectMousePositionToWorld(Origin, Direction);
		const float NearPlaneDist = (Origin - MainEditView->GetLocation()) | MainEditView->GetForward();
		MainEditView->PanCamera(Value.Get<FVector2D>() * ScreenDistToWorldDistScale() * Distance / NearPlaneDist);
	}
}

void AMainEditController::OnZoom(const FInputActionValue& Value)
{
	UE_LOG(LogYoo, Verbose, TEXT("OnZoom %s"), *Value.ToString());
	const float ZoomValue = Value.Get<float>();
	if (bIsInInspectorMode && InspectorCamera)
	{
		InspectorCamera->SetFocalLength(InspectorCamera->FocalLength * (ZoomValue > 0 ? 1.25 : 0.8));
	}
	else
	{
		FHitResult Hit;
		bCursorHit = GetHitResultUnderCursor(ECC_Visibility, true, Hit);
		if (bCursorHit)
		{
			CursorHit = Hit;
		}
		if (MainEditView)
		{
			float Distance = FMath::Clamp(CursorHit.Distance, 1.0, 10000.0);
			FVector TargetCenter = CursorHit.TraceStart + Distance * (CursorHit.TraceEnd - CursorHit.TraceStart).GetSafeNormal();
			MainEditView->ZoomCamera(TargetCenter, ZoomValue > 0 ? 0.8 : 1.25);
		}
	}
}

void AMainEditController::OnLBClicked()
{
	UE_LOG(LogYoo, Verbose, TEXT("OnLBClicked"));
	if (bSelectionHit)
	{
		if (auto Object =Cast<AEditableObject>(SelectionHit.GetActor()))
		{
			if (Object != SelectedObject)
			{
				AttachGizmoTo(Object);
				SelectedObject = Object;
				OnSelectObject(Object);
				OnSelectObjectEvent.Broadcast(Object);
			}
		}
	}
}

void AMainEditController::OnGizmoMove()
{
	SetGizmoMode(GIZMO_MOVE);
}

void AMainEditController::OnGizmoRotate()
{
	SetGizmoMode(GIZMO_ROTATE);
}

void AMainEditController::OnGizmoScale()
{
	SetGizmoMode(GIZMO_SCALE);
}

void AMainEditController::OnConfirm()
{
	if (bIsInPlacementMode)
	{
		ensure(CachedPlacementClass);
		FVector NewLocation = MainEditView->PlacementProxyComponent->GetComponentLocation();

		FActorSpawnParameters SpawnArgs;
		SpawnArgs.OverrideLevel = GetWorld()->GetAuthGameMode<AYooGameMode>()->GetCurrentLevel()->Level;
		
		AEditableObject* NewObject = GetWorld()->SpawnActor<AEditableObject>(CachedPlacementClass, NewLocation, FRotator::ZeroRotator, SpawnArgs);
		CachedOnPlacementFinished.ExecuteIfBound(NewObject);
		OnCreateObject.Broadcast(NewObject);
		MainEditView->PlacementProxyComponent->SetVisibility(false);
		MainEditView->PlacementProxyComponent->SetStaticMesh(nullptr);
		MainEditView->PlacementSpriteComponent->SetVisibility(false);
		MainEditView->PlacementSpriteComponent->SetSprite(nullptr);
		bIsInPlacementMode = false;
		CachedPlacementClass = nullptr;
	}
}

void AMainEditController::OnCancel()
{
	if (bIsInPlacementMode)
	{
		CachedOnPlacementFinished.ExecuteIfBound(nullptr);
		MainEditView->PlacementProxyComponent->SetVisibility(false);
		MainEditView->PlacementProxyComponent->SetStaticMesh(nullptr);
		MainEditView->PlacementSpriteComponent->SetVisibility(false);
		MainEditView->PlacementSpriteComponent->SetSprite(nullptr);
		bIsInPlacementMode = false;
		CachedPlacementClass = nullptr;
	}

	if (bIsInInspectorMode)
	{
		EndInspectCamera();
	}
}

void AMainEditController::OnDelete()
{
	if (SelectedObject)
	{
		SelectedObject->Destroy();
		ReleaseSelection();
	}
}

void AMainEditController::OnMove(const FInputActionValue& Value)
{
	if (bIsInInspectorMode && InspectorCamera)
	{
		FVector MovementVector = Value.Get<FVector>();
	
		const FVector ForwardDirection = InspectorCamera->GetActorForwardVector();
		const FVector RightDirection = InspectorCamera->GetActorRightVector();
		const FVector UpDirection = FVector::UpVector;

		// add movement 
		InspectorCamera->AddInputVector(ForwardDirection * MovementVector.Y);
		InspectorCamera->AddInputVector(RightDirection * MovementVector.X);
		InspectorCamera->AddInputVector(UpDirection * MovementVector.Z);
	}
}

void AMainEditController::OnLook(const FInputActionValue& Value)
{
	if (bIsInInspectorMode && InspectorCamera)
	{
		FVector2D LookAxisVector = Value.Get<FVector2D>();

		// add yaw and pitch input
		RotationInput.Yaw += LookAxisVector.X;
		RotationInput.Pitch += LookAxisVector.Y;
	}
}
