#pragma once

#include "CoreMinimal.h"
#include "EditableObject.h"
#include "CustomSpoutSender.h"
#include "SpoutCamera.generated.h"

UENUM()
enum class ECameraFlag : uint8
{
	None = 0,
	SpoutOutput = 1 << 0,
	Inspector = 1 << 1,
};
ENUM_CLASS_FLAGS(ECameraFlag);

UCLASS()
class YOO_API ASpoutCamera : public AEditorOnlyObject
{
	GENERATED_BODY()

	TRefCountPtr<FSpoutSender> SpoutSender;

public:
	ASpoutCamera(const FObjectInitializer& Initializer);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USceneCaptureComponent2D> CaptureComponent;

	/** The mesh to show visually where the camera is placed */
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UStaticMeshComponent> ProxyMeshComponent;

	UPROPERTY(Transient)
	TObjectPtr<UTextureRenderTarget2D> RenderTarget2D;

	UFUNCTION(BlueprintCallable, BlueprintPure, DisplayName="Get Render Target")
	UTextureRenderTarget* BP_GetRenderTarget() const { return GetRenderTarget(); }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter=SetResolution)
	FIntPoint Resolution;
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FTransform GetCameraTransform() const;

	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly="true"))
	void SetResolution(FIntPoint InResolution);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter=SetFocalLength)
	float FocalLength;

	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly="true"))
	void SetFocalLength(float InFocusLength);
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool GetEnableSpoutOutput();
	
	UFUNCTION(BlueprintCallable)
	void SetEnableSpoutOutput(bool bEnable);
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool GetEnableInspector();
	
	UFUNCTION(BlueprintCallable)
	void SetEnableInspector(bool bEnable);
	
	UFUNCTION(BlueprintCallable)
	void AddInspector();
	
	UFUNCTION(BlueprintCallable)
	void RemoveInspector();

	void HideController() const;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
	virtual void PostLoad() override;

	virtual bool ShouldOutputToSpout() const { return HasAnyFlags(ECameraFlag::SpoutOutput); }
	virtual UTextureRenderTarget* GetRenderTarget() const { return CaptureComponent->TextureTarget; }

	void OnCreateObject(AEditableObject* NewObject) const;
	float GetHorizontalFieldOfView() const;
	bool ShouldCaptureRealtime() const { return HasAnyFlags(ECameraFlag::SpoutOutput | ECameraFlag::Inspector); }
	void UpdatePostProcess() const;

protected:
	UPROPERTY()
	ECameraFlag Flags;

	int32 InspectorCount;

	bool HasAnyFlags(ECameraFlag Contains) const { return EnumHasAnyFlags(Flags, Contains); }
	void SetFlagsValue(ECameraFlag FlagBits, bool bSet) { bSet ? EnumAddFlags(Flags, FlagBits) : EnumRemoveFlags(Flags, FlagBits); }
};

UCLASS()
class YOO_API AProxyCamera : public ASpoutCamera
{
	GENERATED_BODY()
public:
	AProxyCamera(const FObjectInitializer& Initializer);
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UBillboardComponent> SpriteComponent;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<USphereComponent> SphereComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter=SetTargetCamera)
	TObjectPtr<ASpoutCamera> TargetCamera;

	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly="true"))
	void SetTargetCamera(ASpoutCamera* InCamera);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	TArray<ASpoutCamera*> GetAuthorityCameras() const;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	virtual bool ShouldOutputToSpout() const override { return TargetCamera && HasAnyFlags(ECameraFlag::SpoutOutput); }
	virtual UTextureRenderTarget* GetRenderTarget() const override { return TargetCamera ? CaptureComponent->TextureTarget : nullptr; }

	static bool CameraHasAuthority(const ASpoutCamera* InCamera);
};