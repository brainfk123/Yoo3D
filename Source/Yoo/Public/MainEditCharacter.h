// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditableObject.h"
#include "MainEditDataSource.h"
#include "MainEditCharacter.generated.h"

class ULiveLinkRetargetAsset;
class AMainEditDataSource;
class UCapsuleComponent;
class ULiveLinkComponent;

USTRUCT(BlueprintType)
struct FRemapAssetPair
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AMainEditDataSource> DataSourceClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<ULiveLinkRetargetAsset> RetargetAssetClass;
};

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
	TObjectPtr<USkeletalMeshComponent> ShadowProxyMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UCapsuleComponent> Capsule;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDataSourceSubject BodySubject;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDataSourceSubject FaceSubject;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDataSourceSubject HandSubject;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FRemapAssetPair> RetargetAssetList;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	void GetNameAndRetargetAsset(FDataSourceSubject Subject, FName& Name, TSubclassOf<ULiveLinkRetargetAsset>& RetargetAsset);
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual TArray<UPrimitiveComponent*> GetEditorOnlyComponents() const override;
};
