#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "LobbyPlayerController.generated.h"

class ULobbyGameInstance;
class ALobbyGameMode;
class ALobbyFocusLampActor;
class ALobbySlotActor;

class UInputMappingContext;
class UInputAction;

UENUM(BlueprintType)
enum class ELobbyInputOwner : uint8
{
	Unknown  UMETA(DisplayName="Unknown"),
	Mouse    UMETA(DisplayName="Mouse"),
	Gamepad  UMETA(DisplayName="Gamepad"),
	Keyboard UMETA(DisplayName="Keyboard"),
};

UCLASS()
class ALobbyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ALobbyPlayerController();

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void PlayerTick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category="Lobby|Focus")
	int32 GetFocusedSlotIndex() const { return FocusedSlotIndex; }

	UFUNCTION(BlueprintCallable, Category="Lobby|Focus")
	void SetFocusedSlotIndex(int32 NewIndex);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lobby")
	ELobbyInputOwner InputOwner = ELobbyInputOwner::Unknown;

	// ---- Enhanced Input Assets ----
	UPROPERTY(EditDefaultsOnly, Category="Lobby|Input")
	TObjectPtr<UInputMappingContext> LobbyMappingContext = nullptr;

	UPROPERTY(EditDefaultsOnly, Category="Lobby|Input")
	int32 LobbyMappingPriority = 0;

	UPROPERTY(EditDefaultsOnly, Category="Lobby|Input")
	TObjectPtr<UInputAction> IA_FocusLeft = nullptr;

	UPROPERTY(EditDefaultsOnly, Category="Lobby|Input")
	TObjectPtr<UInputAction> IA_FocusRight = nullptr;

	UPROPERTY(EditDefaultsOnly, Category="Lobby|Input")
	TObjectPtr<UInputAction> IA_Confirm = nullptr;

	UPROPERTY(EditDefaultsOnly, Category="Lobby|Input")
	TObjectPtr<UInputAction> IA_Cancel = nullptr;

	UPROPERTY(EditDefaultsOnly, Category="Lobby|Input")
	TObjectPtr<UInputAction> IA_SkinPrev = nullptr;

	UPROPERTY(EditDefaultsOnly, Category="Lobby|Input")
	TObjectPtr<UInputAction> IA_SkinNext = nullptr;

	// ---- Focus lamp ----
	UPROPERTY(EditDefaultsOnly, Category="Lobby|Focus")
	TSubclassOf<ALobbyFocusLampActor> FocusLampClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lobby|Focus")
	FLinearColor FocusLampColor = FLinearColor::White;

protected:
	// Enhanced input handlers
	void OnFocusLeft(const FInputActionValue& Value);
	void OnFocusRight(const FInputActionValue& Value);

	void OnConfirm(const FInputActionValue& Value);
	void OnCancel(const FInputActionValue& Value);

	void OnSkinPrev(const FInputActionValue& Value);
	void OnSkinNext(const FInputActionValue& Value);

private:
	UPROPERTY()
	TObjectPtr<ULobbyGameInstance> LobbyGI;

	UPROPERTY()
	TObjectPtr<ALobbyGameMode> LobbyGM;

	UPROPERTY(EditAnywhere, Category="Lobby|Focus")
	int32 FocusedSlotIndex = 0;

	UPROPERTY()
	TObjectPtr<ALobbyFocusLampActor> FocusLamp = nullptr;

	UPROPERTY()
	TObjectPtr<ALobbySlotActor> HoveredSlot = nullptr;

	UPROPERTY()
	TObjectPtr<ALobbySlotActor> FocusedSlotActor = nullptr;

private:
	void CacheLobbyRefs();

	void EnsureFocusLamp();
	void MoveLampToFocusedSlot();

	void ClampAndApplyFocus();
	int32 WrapSlotIndex(int32 Index) const;

	void UpdateMouseHoverFocus();

	void AddLobbyMappingContext();
};
