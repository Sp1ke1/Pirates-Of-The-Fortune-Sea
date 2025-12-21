#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GDS_2025/Lobby/Data/LobbyTypes.h"
#include "LobbyDeviceRegistry.generated.h"

UCLASS(BlueprintType)
class ULobbyDeviceRegistry : public UObject
{
	GENERATED_BODY()

public:
	// How many gamepad indices we "consider" available for selection.
	// Later you can replace this with actual connected-device detection.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lobby")
	int32 MaxGamepadsToOffer = 4;

	// Rebuild internal list (stub for now, but keeps API clean)
	UFUNCTION(BlueprintCallable, Category="Lobby|Devices")
	void Refresh();

	// Returns options user can choose for controlling a slot.
	// Uses internal reservations to filter out already taken gamepads/keyboard.
	UFUNCTION(BlueprintCallable, Category="Lobby|Devices")
	TArray<FLobbyControlOption> GetAvailableOptions(int32 SlotIndex) const;

	// Reserve a device for a slot (GameInstance should call this after applying assignment)
	UFUNCTION(BlueprintCallable, Category="Lobby|Devices")
	bool ReserveDeviceForSlot(const FLobbyDeviceId& DeviceId, int32 SlotIndex);

	// Release any device reserved by this slot
	UFUNCTION(BlueprintCallable, Category="Lobby|Devices")
	void ReleaseSlot(int32 SlotIndex);

	// Query
	UFUNCTION(BlueprintCallable, Category="Lobby|Devices")
	bool IsDeviceReserved(const FLobbyDeviceId& DeviceId) const;

	UFUNCTION(BlueprintCallable, Category="Lobby|Devices")
	bool IsDeviceReservedByOtherSlot(const FLobbyDeviceId& DeviceId, int32 SlotIndex) const;

private:
	// Device -> SlotIndex
	UPROPERTY()
	TMap<FLobbyDeviceId, int32> ReservedDevices;

private:
	static bool IsReservableDeviceType(ELobbyDeviceType Type);
};
