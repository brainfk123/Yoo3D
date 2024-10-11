#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CustomSpoutSender.h"

#include "EditableObject.generated.h"

class UEditableObjectDetailWidget;

UCLASS()
class YOO_API AEditableObject : public AActor
{
	GENERATED_BODY()
public:
	AEditableObject(const FObjectInitializer& Initializer = FObjectInitializer::Get());
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=EditableObject)
	FString DisplayName;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=EditableObject)
	FText TypeName;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=EditableObject)
	TObjectPtr<UTexture2D> TypeThumbnail;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=EditableObject)
	TObjectPtr<UStaticMesh> ProxyMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=EditableObject)
	bool bProxyMeshToonShade;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=EditableObject)
	TObjectPtr<UTexture2D> ProxySprite;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=EditableObject)
	TSubclassOf<UEditableObjectDetailWidget> DetailWidgetClass;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category=EditableObject)
	TArray<TSubclassOf<UEditableObjectDetailWidget>> GetDetailWidgets();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Velocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSpeed;

	/** Acceleration applied by input (rate of change of velocity) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Acceleration;

	/** Deceleration applied when there is no input (rate of change of velocity) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=FloatingPawnMovement)
	float Deceleration;
	
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void PostActorCreated() override;

	virtual TArray<UPrimitiveComponent*> GetEditorOnlyComponents() const { return {}; }

	void AddInputVector(FVector WorldAccel);

	bool IsExceedingMaxSpeed(float InMaxSpeed) const;

	void UpdateMovement(float DeltaTime);

	void StopMovement();

	FString MakeUniqueName(const FString& BaseName) const;

protected:
	FVector InputVector;
};

UCLASS()
class YOO_API AEditorOnlyObject : public AEditableObject
{
	GENERATED_BODY()
};
