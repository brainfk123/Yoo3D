#pragma once

#include "CoreMinimal.h"
#include "YooGameMode.h"
#include "GameFramework/PlayerController.h"
#include "MainEditController.generated.h"

class ULevelSaveData;
class ASpoutCamera;
class AEditableObject;
struct FInputActionValue;
class UInputAction;
class UInputMappingContext;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSelectObject, AEditableObject*, Object);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnPlacementFinished, AEditableObject*, Object);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnCreateObject, AEditableObject*);

UENUM(BlueprintType)
enum EGizmoMode
{
	GIZMO_MOVE UMETA(DisplayName = "Move Mode"),
	GIZMO_ROTATE UMETA(DisplayName = "Rotate Mode"),
	GIZMO_SCALE UMETA(DisplayName = "Scale Mode")
};

UCLASS()
class YOO_API AMainEditController : public APlayerController
{
	GENERATED_BODY()
public:
	AMainEditController();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Game)
	TSubclassOf<UUserWidget> MainHUDClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Game)
	TObjectPtr<UUserWidget> MainHUDWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Game)
	TSubclassOf<UUserWidget> CameraInspectorClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Game)
	TObjectPtr<UUserWidget> CameraInspectorWidget;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	TObjectPtr<UInputAction> RotateAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	TObjectPtr<UInputAction> PanAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	TObjectPtr<UInputAction> ZoomAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	TObjectPtr<UInputAction> LBPressedAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	TObjectPtr<UInputAction> RBPressedAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	TObjectPtr<UInputAction> LBReleasedAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	TObjectPtr<UInputAction> RBReleasedAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	TObjectPtr<UInputAction> LBClickAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	TObjectPtr<UInputAction> GizmoMoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	TObjectPtr<UInputAction> GizmoRotateAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	TObjectPtr<UInputAction> GizmoScaleAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	TObjectPtr<UInputAction> ConfirmAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	TObjectPtr<UInputAction> CancelAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	TObjectPtr<UInputAction> DeleteAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	TObjectPtr<UInputMappingContext> InspectorMappingContext;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* LookAction;

	UPROPERTY(Transient)
	FHitResult CursorHit;
	bool bCursorHit;
	
	UPROPERTY(Transient)
	FHitResult SelectionHit;
	bool bSelectionHit;

	UPROPERTY(Transient)
	TObjectPtr<class AMainEditView> MainEditView;

	UPROPERTY(Transient)
	TObjectPtr<AEditableObject> SelectedObject;

	UPROPERTY(BlueprintAssignable, Category=Editor)
	FOnSelectObject OnSelectObjectEvent;

	UFUNCTION(BlueprintImplementableEvent, Category=Editor)
	void OnSelectObject(AEditableObject* Object);
	
	FOnCreateObject OnCreateObject;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category=Editor)
	void SelectObject(AEditableObject* Object);

	UFUNCTION(BlueprintCallable, Category=Editor)
	void ReleaseSelection();

	UFUNCTION(BlueprintCallable, Category=Editor)
	void BeginPlaceNewObject(TSubclassOf<AEditableObject> InClass, FOnPlacementFinished OnPlacementFinished);
	
	UPROPERTY()
	TSubclassOf<AEditableObject> CachedPlacementClass;
	
	UPROPERTY()
	FOnPlacementFinished CachedOnPlacementFinished;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Game)
	TSubclassOf<AActor> GizmoClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Game)
	TObjectPtr<AActor> Gizmo;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = Game)
	void AttachGizmoTo(AEditableObject* Object);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = Game)
	void SetGizmoMode(EGizmoMode NewMode);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = Game)
	void DetachGizmo();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = Game)
	void UpdateGizmo();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Game)
	AEditableObject* GetSelectedObject();

	UFUNCTION(BlueprintCallable, Category = Game)
	void SetSelectedActorTransform(const FTransform& NewTransform);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Game)
	TObjectPtr<ASpoutCamera> InspectorCamera;

	UFUNCTION(BlueprintCallable, Category = Game)
	void BeginInspectCamera(ASpoutCamera* Camera);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = Game)
	void SetInspectCameraToWidget(UUserWidget* Widget, ASpoutCamera* Camera);

	UFUNCTION(BlueprintCallable, Category = Game)
	void EndInspectCamera();

	UFUNCTION(BlueprintCallable, Category = Game)
	void SetMainCamera(ASpoutCamera* InCamera);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Game)
	ASpoutCamera* GetMainCamera();
	
	virtual void SetupInputComponent() override;
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	float ScreenDistToWorldDistScale() const;
	FVector GetPlacementHitLocation() const;
	void UpdateInspectorRotation();
	void LoadFromLevelData(ULevelSaveData* NewLevel) const;
	void SaveToLevelData(ULevelSaveData* NewLevel) const;
	void SetCharacterLight(float Intensity, FColor LightColor) const;

	/** Input handlers for SetDestination action. */
	void OnLBPressed();
	void OnRBPressed();
	void OnLBReleased();
	void OnRBReleased();
	void OnRotate(const FInputActionValue& Value);
	void OnPan(const FInputActionValue& Value);
	void OnZoom(const FInputActionValue& Value);
	void OnLBClicked();
	void OnGizmoMove();
	void OnGizmoRotate();
	void OnGizmoScale();
	void OnConfirm();
	void OnCancel();
	void OnDelete();
	void OnMove(const FInputActionValue& Value);
	void OnLook(const FInputActionValue& Value);

private:
	bool bIsInPlacementMode;
	bool bIsInInspectorMode;
	FRotator RotationInput;
};
