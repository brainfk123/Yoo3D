#include "EditableObject.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "EngineUtils.h"
#include "MainEditController.h"
#include "EditableObjectDetailWidget.h"

#define LOCTEXT_NAMESPACE "EditableObject"

AEditableObject::AEditableObject(const FObjectInitializer& Initializer)
	: Super(Initializer)
{
	TypeName = FText::FromString(GetClass()->GetName());
	
	if (GetClass() == StaticClass())
	{
		static ConstructorHelpers::FClassFinder<UEditableObjectDetailWidget> TransformWidget(TEXT("/Game/NewEditor/Blueprints/Widgets/ObjectDetails/Common/UI_TransformDetail"));
		DetailWidgetClass = TransformWidget.Class;
	}

	PrimaryActorTick.bCanEverTick = true;
	MaxSpeed = 1200.f;
	Acceleration = 4000.f;
	Deceleration = 8000.f;
	bProxyMeshToonShade = false;
}

TArray<TSubclassOf<UEditableObjectDetailWidget>> AEditableObject::GetDetailWidgets()
{
	TArray<TSubclassOf<UEditableObjectDetailWidget>> WidgetList;

	for (TSubclassOf<AEditableObject> ObjectClass(GetClass()); ObjectClass && ObjectClass->IsChildOf<AEditableObject>(); ObjectClass = ObjectClass->GetSuperClass())
	{
		if (ObjectClass.GetDefaultObject()->DetailWidgetClass)
		{
			WidgetList.AddUnique(ObjectClass.GetDefaultObject()->DetailWidgetClass);
		}
	}

	Algo::Reverse(WidgetList);
	return WidgetList;
}

void AEditableObject::BeginPlay()
{
	Super::BeginPlay();
	TArray<UPrimitiveComponent*> Components;
	GetComponents(Components);
	for (auto Component : Components)
	{
		Component->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);
	}
}

void AEditableObject::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateMovement(DeltaSeconds);
}

void AEditableObject::PostActorCreated()
{
	DisplayName = MakeUniqueName(TypeName.ToString());
}

void AEditableObject::AddInputVector(FVector WorldAccel)
{
	InputVector += WorldAccel;
}

bool AEditableObject::IsExceedingMaxSpeed(float InMaxSpeed) const
{
	InMaxSpeed = FMath::Max(0.f, InMaxSpeed);
	const float MaxSpeedSquared = FMath::Square(InMaxSpeed);
	
	// Allow 1% error tolerance, to account for numeric imprecision.
	const float OverVelocityPercent = 1.01f;
	return (Velocity.SizeSquared() > MaxSpeedSquared * OverVelocityPercent);
}

void AEditableObject::UpdateMovement(float DeltaTime)
{
	const FVector ControlAcceleration = InputVector.GetClampedToMaxSize(1.f);

	const float AnalogInputModifier = (ControlAcceleration.SizeSquared() > 0.f ? ControlAcceleration.Size() : 0.f);
	const float MaxPawnSpeed = MaxSpeed * AnalogInputModifier;
	const bool bExceedingMaxSpeed = IsExceedingMaxSpeed(MaxPawnSpeed);

	if (AnalogInputModifier > 0.f && !bExceedingMaxSpeed)
	{
		// Apply change in velocity direction
		if (Velocity.SizeSquared() > 0.f)
		{
			// Change direction faster than only using acceleration, but never increase velocity magnitude.
			const float TimeScale = FMath::Clamp(DeltaTime, 0.f, 1.f);
			Velocity = Velocity + (ControlAcceleration * Velocity.Size() - Velocity) * TimeScale;
		}
	}
	else
	{
		// Dampen velocity magnitude based on deceleration.
		if (Velocity.SizeSquared() > 0.f)
		{
			const FVector OldVelocity = Velocity;
			const float VelSize = FMath::Max(Velocity.Size() - FMath::Abs(Deceleration) * DeltaTime, 0.f);
			Velocity = Velocity.GetSafeNormal() * VelSize;

			// Don't allow braking to lower us below max speed if we started above it.
			if (bExceedingMaxSpeed && Velocity.SizeSquared() < FMath::Square(MaxPawnSpeed))
			{
				Velocity = OldVelocity.GetSafeNormal() * MaxPawnSpeed;
			}
		}
	}

	// Apply acceleration and clamp velocity magnitude.
	const float NewMaxSpeed = (IsExceedingMaxSpeed(MaxPawnSpeed)) ? Velocity.Size() : MaxPawnSpeed;
	Velocity += ControlAcceleration * FMath::Abs(Acceleration) * DeltaTime;
	Velocity = Velocity.GetClampedToMaxSize(NewMaxSpeed);

	AddActorWorldOffset(Velocity * DeltaTime);

	InputVector = FVector::ZeroVector;
}

void AEditableObject::StopMovement()
{
	InputVector = FVector::ZeroVector;
	Velocity = FVector::ZeroVector;
}

FString AEditableObject::MakeUniqueName(const FString& BaseName) const
{
	if (!GetWorld())
		return BaseName;
	
	TSet<FString> NameList;
	for (TActorIterator<AEditableObject> It(GetWorld()); It; ++It)
	{
		NameList.Add(It->DisplayName);
	}

	if (!NameList.Contains(BaseName))
		return BaseName;
	
	int32 Counter = 1;
	FString NewName = BaseName;
	
	while (true)
	{
		FString TestName = FString::Printf(TEXT("%s %d"), *BaseName, Counter++);
		if (!NameList.Contains(TestName))
		{
			NewName = TestName;
			break;
		}
	}
	
	return NewName;
}

#undef LOCTEXT_NAMESPACE
