#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GDS_2025/Lobby/Data/LobbyTypes.h"
#include "LobbyDeviceRegistry.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnLobbyDevicesChangedNative);

UCLASS()
class ULobbyDeviceRegistry : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, Category="Lobby|Devices")
	int32 MaxSupportedGamepads = 4;

	FOnLobbyDevicesChangedNative OnDevicesChangedNative;

	// Rebuilds connected device list (best-effort) and broadcasts if changed.
	UFUNCTION(BlueprintCallable, Category="Lobby|Devices")
	void Refresh();

	// Builds hidden list of available assignments right now.
	// Stable order: None, Keyboard (if free), Gamepads (free), AI
	UFUNCTION(BlueprintCallable, Category="Lobby|Devices")
	void BuildAssignableOptions(TArray<FLobbyControlAssignment>& OutOptions) const;

	// Checks if an assignment is still valid (device still present / keyboard available).
	UFUNCTION(BlueprintCallable, Category="Lobby|Devices")
	bool IsAssignmentValid(const FLobbyControlAssignment& A) const;

	// Reserves a physical device for a slot (used by GameInstance SetSlotControl).
	// For non-physical assignments (None/AI/Matchmaking): no reservation required, returns true.
	UFUNCTION(BlueprintCallable, Category="Lobby|Devices")
	bool ReserveDeviceForSlot(const FLobbyDeviceId& DeviceId, int32 SlotIndex);

	// Releases any reservation held by this slot.
	UFUNCTION(BlueprintCallable, Category="Lobby|Devices")
	void ReleaseSlot(int32 SlotIndex);

	// Display helper for ALobbySlotActor ChangeText.
	UFUNCTION(BlueprintCallable, Category="Lobby|Devices")
	FText ToDisplayText(const FLobbyControlAssignment& A) const;

	UFUNCTION(BlueprintCallable, Category="Lobby|Devices")
	bool IsKeyboardAvailable() const { return bKeyboardAvailable; }

	UFUNCTION(BlueprintCallable, Category="Lobby|Devices")
	TArray<int32> GetConnectedGamepadIndices() const { return ConnectedGamepadIndices; }

private:
	// Connected gamepad indices (0,1,2,...) as seen by the platform enumerator / mapper.
	UPROPERTY()
	TArray<int32> ConnectedGamepadIndices;

	// Device reservations: DeviceId -> SlotIndex
	UPROPERTY()
	TMap<FLobbyDeviceId, int32> ReservedByDeviceId;

	// SlotIndex -> DeviceId
	UPROPERTY()
	TMap<int32, FLobbyDeviceId> ReservedBySlot;

	UPROPERTY()
	bool bKeyboardAvailable = true;

private:
	void QueryConnectedGamepads(TArray<int32>& OutGamepadIndices) const;

	bool IsGamepadConnectedIndex(int32 GamepadIndex) const;
	int32 GetGamepadDisplayNumber(int32 GamepadIndex) const;
};
