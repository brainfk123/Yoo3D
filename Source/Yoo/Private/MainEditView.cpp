#include "MainEditView.h"
#include "SpoutCamera.h"
#include "Camera/CameraComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/DirectionalLightComponent.h"

// Sets default values
AMainEditView::AMainEditView()
{
	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Create a camera...
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("MainEditView"));
	CameraComponent->SetupAttachment(RootComponent);

	CharacterLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("Character Light"));
	CharacterLight->SetupAttachment(RootComponent);
	CharacterLight->LightingChannels.bChannel0 = false;
	CharacterLight->LightingChannels.bChannel1 = true;
	CharacterLight->Intensity = 2.0f;
	CharacterLight->LightColor = FColor::White;
	CharacterLight->ForwardShadingPriority = 10;
	CharacterLight->bAtmosphereSunLight = false;
	
	PlacementProxyComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlacementProxyComponent"));
	PlacementProxyComponent->SetupAttachment(RootComponent);
	PlacementProxyComponent->SetVisibility(false);
	PlacementProxyComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	PlacementProxyComponent->CastShadow = false;

	PlacementSpriteComponent = CreateDefaultSubobject<UBillboardComponent>(TEXT("PlacementSpriteComponent"));
	PlacementSpriteComponent->SetupAttachment(PlacementProxyComponent);
	PlacementSpriteComponent->SetVisibility(false);
	PlacementSpriteComponent->CastShadow = false;
	PlacementSpriteComponent->bHiddenInGame = false;

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	RotationSpeed = 1.0;
	PanSpeed = 15.0;
	ZoomSpeed = 1.0;
}

void AMainEditView::SetCameraRotation(const FRotator& NewRotation)
{
	CameraComponent->SetRelativeRotation(NewRotation);
}

void AMainEditView::SetCameraTransform(const FTransform& NewTransform)
{
	CameraComponent->SetWorldTransform(NewTransform);
}

FTransform AMainEditView::GetCameraTransform()
{
	return CameraComponent->GetComponentTransform();
}

// Called when the game starts or when spawned
void AMainEditView::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AMainEditView::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AMainEditView::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AMainEditView::RotateCamera(const FVector& Target, const FRotator& BaseRotation)
{
	const FRotator OldRotation = CameraComponent->GetRelativeRotation();
	FRotator NewRotation = OldRotation + BaseRotation * RotationSpeed;
	NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch, -89.9, 89.9);
	const FRotator Rotator = NewRotation - OldRotation;

	const FVector CameraBoom = Target - GetActorLocation();
	FVector NewCameraBoom = CameraBoom.RotateAngleAxis(Rotator.Yaw, FVector::UpVector);
	NewCameraBoom = NewCameraBoom.RotateAngleAxis(Rotator.Pitch, GetActorForwardVector() ^ FVector::UpVector);
	SetActorLocation(Target - NewCameraBoom);
	CameraComponent->SetRelativeRotation(NewRotation);
}

void AMainEditView::PanCamera(const FVector2D& Offset)
{
	const FVector FinalOffset = CameraComponent->GetRightVector() * Offset.X - CameraComponent->GetUpVector() * Offset.Y;
	AddActorWorldOffset(FinalOffset * PanSpeed);
}

void AMainEditView::ZoomCamera(const FVector& Target, float Scale)
{
	FVector CameraBoom = GetActorLocation() - Target;
	Scale = 1.0 + (Scale - 1.0f) * ZoomSpeed;
	SetActorLocation(Target + CameraBoom * Scale);
}

void AMainEditView::UpdateCharacterLight()
{
	if (MainCamera)
	{
		CharacterLight->SetWorldTransform(MainCamera->GetCameraTransform());
	}
	else
	{
		CharacterLight->SetWorldTransform(GetCameraTransform());
	}
}

FVector AMainEditView::GetForward() const
{
	return CameraComponent->GetForwardVector();
}

FVector AMainEditView::GetLocation() const
{
	return GetActorLocation();
}

