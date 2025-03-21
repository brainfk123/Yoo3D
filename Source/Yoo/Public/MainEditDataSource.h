﻿#pragma once

#include "CoreMinimal.h"
#include "EditableObject.h"
#include "NeuronLiveLinkSource.h"
#include "Interfaces/IPv4/IPv4Endpoint.h"
#include "Components/BillboardComponent.h"
#include "Components/SphereComponent.h"
#include "MainEditDataSource.generated.h"

class AMainEditDataSource;
class FAppleARKitLiveLinkSource;

UENUM(BlueprintType)
enum class EDataSourceState : uint8
{
	Unknown,
	Online,
	Offline,
	Error
};

UENUM(BlueprintType)
enum class EDataSourceType : uint8
{
	Unknown = 0,
	Body = 1 << 0,
	Face = 1 << 1,
	Hand = 1 << 2,

	Body_Face	= Body | Face			UMETA(DisplayName="Body & Face"),
	Body_Hand	= Body | Hand			UMETA(DisplayName="Body & Hand"),
	Face_Hand	= Face | Hand			UMETA(DisplayName="Face & Hand"),
	All			= Body | Face | Hand	UMETA(DisplayName="All"),
};
ENUM_CLASS_FLAGS(EDataSourceType)

namespace DataSourceType
{
	inline bool HasFlags(EDataSourceType Type, EDataSourceType Flag) { return (Type & Flag) == Flag; }
	inline void SetFlag(EDataSourceType& Type, EDataSourceType Flag) { Type |= Flag; }
	inline void ClearFlag(EDataSourceType& Type, EDataSourceType Flag) { Type &= ~Flag; }
}

USTRUCT(BlueprintType)
struct FDataSourceSubject
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText TypeName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SubjectName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AMainEditDataSource> TypeClass;
};

UCLASS(Abstract)
class YOO_API AMainEditDataSource : public AEditorOnlyObject
{
	GENERATED_BODY()
public:
	AMainEditDataSource();
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UBillboardComponent> SpriteComponent;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<USphereComponent> SphereComponent;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Data Source")
	EDataSourceState CurrentStatus() const { return CurrentStatus_Implement(); }

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EDataSourceType Type;

	virtual TArray<FName> GetSubjectNames() const PURE_VIRTUAL(AMainEditDataSource::GetSubjectNames, return {};)

protected:
	virtual EDataSourceState CurrentStatus_Implement() const PURE_VIRTUAL(AMainEditDataSource::CurrentState_Implement, return EDataSourceState::Unknown;);
};

UCLASS(MinimalAPI)
class ANeuronDataSource : public AMainEditDataSource
{
	GENERATED_BODY()

public:
	ANeuronDataSource();
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Data Source")
	static bool IsLegalIPString(const FString& String);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Data Source")
	FString GetLocalIPString() const;

	UFUNCTION(BlueprintCallable, Category="Data Source")
	bool SetLocalIPString(const FString& String);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Data Source", DisplayName="Get Subject Names")
	TArray<FName> BP_GetSubjectNames() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Data Source")
	float GetSkeletonHeight(FName SubjectName);

	UFUNCTION(BlueprintCallable, Category="Data Source")
	void SetSkeletonHeight(FName SubjectName, float Height);

	virtual TArray<FName> GetSubjectNames() const override;
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Serialize(FArchive& Ar) override;

	virtual EDataSourceState CurrentStatus_Implement() const override;

	void UpdateSource();
	void DestroySource();

private:
	TSharedPtr<FNeuronLiveLinkSource> NeuronSource;
	FIPv4Endpoint LocalEndpoint;
	TMap<FName, float> SubjectNameToSkeletonHeight;
};

USTRUCT(BlueprintType)
struct FARKitSubject
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SubjectName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName DeviceName;
};

UCLASS(MinimalAPI)
class AARKitDataSource : public AMainEditDataSource
{
	GENERATED_BODY()

public:
	AARKitDataSource();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Data Source")
	TArray<FARKitSubject> GetSubjectAndDeviceNames() const;
	
	virtual TArray<FName> GetSubjectNames() const override;
protected:
	virtual void BeginPlay() override;

	virtual EDataSourceState CurrentStatus_Implement() const override;

private:
	FAppleARKitLiveLinkSource* ARKitSource;
};
