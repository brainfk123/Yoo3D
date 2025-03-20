#include "MainEditCharacter.h"

#include "LiveLinkRemapAsset.h"
#include "LiveLinkRetargetAsset.h"
#include "Components/CapsuleComponent.h"


// Sets default values
AMainEditCharacter::AMainEditCharacter()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
	
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Skeleton Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->LightingChannels.bChannel0 = false;
	Mesh->LightingChannels.bChannel1 = true;
	
	ShadowProxyMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Shadow Proxy Mesh"));
	ShadowProxyMesh->SetupAttachment(Mesh);
	ShadowProxyMesh->bCastHiddenShadow = true;
	ShadowProxyMesh->SetVisibility(false);
	
	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Capsule->SetupAttachment(Mesh);
	Capsule->SetRelativeLocation(FVector(0.0, 0.0, Capsule->GetScaledCapsuleHalfHeight()));
	Capsule->bHiddenInGame = false;
	Capsule->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}

void AMainEditCharacter::GetNameAndRetargetAsset(FDataSourceSubject Subject, FName& Name, TSubclassOf<ULiveLinkRetargetAsset>& RetargetAsset)
{
	Name = Subject.SubjectName;
	RetargetAsset = ULiveLinkRemapAsset::StaticClass();

	if (Subject.TypeClass)
	{
		for (const auto& [DataSourceClass, RetargetAssetClass] : RetargetAssetList)
		{
			if (Subject.TypeClass->IsChildOf(DataSourceClass))
			{
				RetargetAsset = RetargetAssetClass;
				return;
			}
		}
	}
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

