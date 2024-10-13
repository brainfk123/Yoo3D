#include "MainEditCharacter.h"
#include "Components/CapsuleComponent.h"


// Sets default values
AMainEditCharacter::AMainEditCharacter()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	MotionSubjectName = NAME_None;
	FaceSubjectName = NAME_None;
	
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
	
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Skeleton Mesh"));
	Mesh->SetupAttachment(RootComponent);
	
	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Capsule->SetupAttachment(Mesh);
	Capsule->SetRelativeLocation(FVector(0.0, 0.0, Capsule->GetScaledCapsuleHalfHeight()));
	Capsule->bHiddenInGame = false;
}

// Called when the game starts or when spawned
void AMainEditCharacter::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AMainEditCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

TArray<UPrimitiveComponent*> AMainEditCharacter::GetEditorOnlyComponents() const
{
	return {Capsule};
}

