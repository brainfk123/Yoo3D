#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MainEditView.generated.h"

UCLASS()
class YOO_API AMainEditView : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMainEditView();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Editor)
	class UCameraComponent* CameraComponent;

	UPROPERTY(VisibleAnywhere, Category = Editor)
	TObjectPtr<class UStaticMeshComponent> PlacementProxyComponent;
	
	UPROPERTY(VisibleAnywhere, Category = Editor)
	TObjectPtr<class UBillboardComponent> PlacementSpriteComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Editor)
	float RotationSpeed;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Editor)
	float PanSpeed;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Editor)
	float ZoomSpeed;

	UFUNCTION(BlueprintCallable, Category = Editor)
	void SetCameraRotation(const FRotator& NewRotation);

	UFUNCTION(BlueprintCallable, Category = Editor)
	void SetCameraTransform(const FTransform& NewTransform);

	UFUNCTION(BlueprintCallable, Category = Editor)
	FTransform GetCameraTransform();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void RotateCamera(const FVector& Target, const FRotator& BaseRotation);
	void PanCamera(const FVector2D& Offset);
	void ZoomCamera(const FVector& Target, float Scale);

	FVector GetForward() const;
	FVector GetLocation() const;
};
