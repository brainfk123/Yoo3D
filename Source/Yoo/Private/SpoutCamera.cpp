#include "SpoutCamera.h"
#include "MainEditController.h"

#define LOCTEXT_NAMESPACE "EditableObject"

static FName ProxyMeshComponentName(TEXT("CameraMesh"));

ASpoutCamera::ASpoutCamera(const FObjectInitializer& Initializer)
	: Super(Initializer)
{
	TypeName = LOCTEXT("ASpoutCamera_TypeName", "Camera");

	Resolution = {1920, 1080};
	Flags = ECameraFlag::None;
	FocalLength = 35.0f;
	InspectorCount = 0;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent")));

	CaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("CaptureComponent"));
	CaptureComponent->SetupAttachment(RootComponent);
	
	ProxyMeshComponent = CreateOptionalDefaultSubobject<UStaticMeshComponent>(ProxyMeshComponentName);
	if (ProxyMeshComponent)
	{
		ProxyMeshComponent->SetupAttachment(CaptureComponent);
		ProxyMeshComponent->SetStaticMesh(ProxyMesh);
		ProxyMeshComponent->CastShadow = false;
		ProxyMeshComponent->SetVisibility(false);
	}
}

void ASpoutCamera::SetResolution(FIntPoint InResolution)
{
	Resolution = InResolution;
	RenderTarget2D->ResizeTarget(InResolution.X, InResolution.Y);
}

void ASpoutCamera::SetFocalLength(float InFocusLength)
{
	FocalLength = FMath::Clamp(InFocusLength, 6.0, 1200.0);
	CaptureComponent->FOVAngle = GetHorizontalFieldOfView();
}

bool ASpoutCamera::GetEnableSpoutOutput()
{
	return HasAnyFlags(ECameraFlag::SpoutOutput);
}

void ASpoutCamera::SetEnableSpoutOutput(bool bEnable)
{
	SetFlagsValue(ECameraFlag::SpoutOutput, bEnable);
}

bool ASpoutCamera::GetEnableInspector()
{
	return HasAnyFlags(ECameraFlag::Inspector);
}

void ASpoutCamera::SetEnableInspector(bool bEnable)
{
	SetFlagsValue(ECameraFlag::Inspector, bEnable);
}

void ASpoutCamera::AddInspector()
{
	InspectorCount++;
	SetEnableInspector(InspectorCount > 0);
}

void ASpoutCamera::RemoveInspector()
{
	InspectorCount = FMath::Max(InspectorCount - 1, 0);
	SetEnableInspector(InspectorCount > 0);
}

void ASpoutCamera::HideController() const
{
	if (auto Controller = GetWorld()->GetFirstPlayerController<AMainEditController>())
	{
		CaptureComponent->HideActorComponents(Controller);
		CaptureComponent->HideActorComponents(Controller->GetPawn());
		CaptureComponent->HideActorComponents(Controller->Gizmo);
	}
}

void ASpoutCamera::BeginPlay()
{
	Super::BeginPlay();
	
	RenderTarget2D = NewObject<UTextureRenderTarget2D>();
	RenderTarget2D->RenderTargetFormat = RTF_RGBA8;
	RenderTarget2D->TargetGamma = 2.2f;
	RenderTarget2D->InitAutoFormat(Resolution.X, Resolution.Y);
	
	CaptureComponent->FOVAngle = GetHorizontalFieldOfView();
	CaptureComponent->CaptureSource = SCS_FinalColorLDR;
	CaptureComponent->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
	CaptureComponent->bAlwaysPersistRenderingState = true;
	CaptureComponent->ShowFlags.TemporalAA = true;
	CaptureComponent->ShowFlags.MotionBlur = true;
	CaptureComponent->TextureTarget = RenderTarget2D;
	
	for (TActorIterator<AEditableObject> It(GetWorld()); It; ++It)
	{
		if (It->IsA(AEditorOnlyObject::StaticClass()))
		{
			CaptureComponent->HideActorComponents(*It);
		}
		else
		{
			for (const auto Component : It->GetEditorOnlyComponents())
			{
				CaptureComponent->HideComponent(Component);
			}
		}
	}

	if (ProxyMeshComponent)
	{
		ProxyMeshComponent->SetStaticMesh(ProxyMesh);
		ProxyMeshComponent->SetVisibility(true);
	}

	if (auto Controller = GetWorld()->GetFirstPlayerController<AMainEditController>())
	{
		Controller->OnCreateObject.AddUObject(this, &ASpoutCamera::OnCreateObject);
		FTimerHandle MyTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(MyTimerHandle, this, &ASpoutCamera::HideController, 0.2f);
	}
}

void ASpoutCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CaptureComponent->bCaptureEveryFrame = ShouldCaptureRealtime();

	if (ShouldOutputToSpout())
	{
		if (!SpoutSender || !SpoutSender->IsCompatibleWith(DisplayName, RenderTarget2D))
		{
			SpoutSender = new FSpoutSender(DisplayName, RenderTarget2D->SizeX, RenderTarget2D->SizeY, RenderTarget2D->GetFormat());
		}
		SpoutSender->SendTexture(RenderTarget2D);
	}
	else
	{
		SpoutSender.SafeRelease();
	}
}

void ASpoutCamera::PostLoad()
{
	Super::PostLoad();
	SetFlagsValue(ECameraFlag::Inspector, false);
}

void ASpoutCamera::OnCreateObject(AEditableObject* NewObject) const
{
	if (NewObject->IsA(AEditorOnlyObject::StaticClass()))
	{
		CaptureComponent->HideActorComponents(NewObject);
	}
	else
	{
		for (const auto Component : NewObject->GetEditorOnlyComponents())
		{
			CaptureComponent->HideComponent(Component);
		}
	}
}

float ASpoutCamera::GetHorizontalFieldOfView() const
{
	if (FocalLength > 0.f)
	{
		constexpr float SensorWidth = 36.0f;
		return FMath::RadiansToDegrees(2.f * FMath::Atan(SensorWidth / (2.f * FocalLength)));
	}

	return 0.f;
}

AProxyCamera::AProxyCamera(const FObjectInitializer& Initializer)
	: Super(Initializer.DoNotCreateDefaultSubobject(ProxyMeshComponentName))
{
	TypeName = LOCTEXT("AProxyCamera_TypeName", "Proxy Camera");
	
	SpriteComponent = CreateDefaultSubobject<UBillboardComponent>(TEXT("SpriteComponent"));
	SpriteComponent->SetupAttachment(RootComponent);
	SpriteComponent->SetVisibility(true);
	SpriteComponent->CastShadow = false;
	SpriteComponent->bHiddenInGame = false;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->SetupAttachment(SpriteComponent);
	SphereComponent->SetSphereRadius(24.0f);
}

void AProxyCamera::SetTargetCamera(ASpoutCamera* InCamera)
{
	TargetCamera = CameraHasAuthority(InCamera) ? InCamera : nullptr;
}

TArray<ASpoutCamera*> AProxyCamera::GetAuthorityCameras() const
{
	TArray<ASpoutCamera*> Result;
	for (TActorIterator<ASpoutCamera> It(GetWorld()); It; ++It)
	{
		if (CameraHasAuthority(*It))
		{
			Result.AddUnique(*It);
		}
	}
	return Result;
}

void AProxyCamera::BeginPlay()
{
	Super::BeginPlay();

	SpriteComponent->SetSprite(ProxySprite);
}

void AProxyCamera::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (TargetCamera)
	{
		SetFocalLength(TargetCamera->FocalLength);
		CaptureComponent->SetWorldTransform(TargetCamera->GetActorTransform());
	}
}

bool AProxyCamera::CameraHasAuthority(const ASpoutCamera* InCamera)
{
	return InCamera && !InCamera->GetClass()->IsChildOf<AProxyCamera>();
}

#undef LOCTEXT_NAMESPACE
