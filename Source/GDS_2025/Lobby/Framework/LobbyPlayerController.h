#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LobbyPlayerController.generated.h"

class ULobbyGameInstance;
class ALobbyGameMode;
class ALobbyFocusLampActor;
class ALobbySlotActor;

// Optional: represent "who owns this input focus"
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

	// Focus
	UFUNCTION(BlueprintCallable, Category="Lobby|Focus")
	int32 GetFocusedSlotIndex() const { return FocusedSlotIndex; }

	UFUNCTION(BlueprintCallable, Category="Lobby|Focus")
	void SetFocusedSlotIndex(int32 NewIndex);

	// Input owner (you can set this when spawning local players)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lobby")
	ELobbyInputOwner InputOwner = ELobbyInputOwner::Unknown;

	// Lamp class to spawn for this controller (set in BP or defaults)
	UPROPERTY(EditDefaultsOnly, Category="Lobby|Focus")
	TSubclassOf<ALobbyFocusLampActor> FocusLampClass;

	// Lamp color (each controller can have its own)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lobby|Focus")
	FLinearColor FocusLampColor = FLinearColor::White;

protected:
	// ---- Input handlers (legacy input system) ----
	void Input_FocusLeft();
	void Input_FocusRight();

	void Input_Confirm(); // A
	void Input_Cancel();  // B (stub)

	void Input_SkinPrev(); // LB
	void Input_SkinNext(); // RB

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

private:
	void CacheLobbyRefs();

	void EnsureFocusLamp();
	void MoveLampToFocusedSlot();

	void ClampAndApplyFocus();
	int32 WrapSlotIndex(int32 Index) const;

	void UpdateMouseHoverFocus();
};
