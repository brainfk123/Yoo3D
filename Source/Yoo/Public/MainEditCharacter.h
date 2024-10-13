// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditableObject.h"
#include "MainEditCharacter.generated.h"

class UCapsuleComponent;
class ULiveLinkComponent;

UCLASS()
class YOO_API AMainEditCharacter : public AEditableObject
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMainEditCharacter();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> Mesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UCapsuleComponent> Capsule;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MotionSubjectName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName FaceSubjectName;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual TArray<UPrimitiveComponent*> GetEditorOnlyComponents() const override;
};
