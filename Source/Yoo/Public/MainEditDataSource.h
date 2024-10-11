#pragma once

#include "CoreMinimal.h"
#include "EditableObject.h"
#include "NeuronLiveLinkSource.h"
#include "Interfaces/IPv4/IPv4Endpoint.h"
#include "MainEditDataSource.generated.h"

class FAppleARKitLiveLinkSource;

UENUM(BlueprintType)
enum class EDataSourceState : uint8
{
	Unknown,
	Online,
	Offline,
	Error
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
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	EDataSourceState CurrentStatus() const { return CurrentStatus_Implement(); }

protected:
	virtual EDataSourceState CurrentStatus_Implement() const PURE_VIRTUAL(AMainEditDataSource::CurrentState_Implement, return EDataSourceState::Unknown;);
};

UCLASS(MinimalAPI)
class ANeuronDataSource : public AMainEditDataSource
{
	GENERATED_BODY()

public:
	ANeuronDataSource();
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static bool IsLegalIPString(const FString& String);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FString GetLocalIPString() const;

	UFUNCTION(BlueprintCallable)
	bool SetLocalIPString(const FString& String);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	TArray<FName> GetSubjectNames() const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetSkeletonHeight(FName SubjectName);

	UFUNCTION(BlueprintCallable)
	void SetSkeletonHeight(FName SubjectName, float Height);

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

	UFUNCTION(BlueprintCallable, BlueprintPure)
	TArray<FARKitSubject> GetSubjectAndDeviceNames() const;

protected:
	virtual void BeginPlay() override;

	virtual EDataSourceState CurrentStatus_Implement() const override;

private:
	FAppleARKitLiveLinkSource* ARKitSource;
};
