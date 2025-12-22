#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GDS_2025/Lobby/Data/LobbySlotData.h"
#include "LobbyGameInstance.generated.h"

class ULobbyDeviceRegistry;
class UPresetPackDataAsset;
class ALobbyPlayerController;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnLobbySlotChangedNative, int32, const FLobbySlotData&);

UCLASS()
class ULobbyGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	static constexpr int32 NumLobbySlots = 4;

	// SlotIndex, NewSlotData
	FOnLobbySlotChangedNative OnLobbySlotChangedNative;

	virtual void Init() override;

	// Data access
	UFUNCTION(BlueprintCallable, Category="Lobby")
	const FLobbySlotData& GetSlotData(int32 SlotIndex) const;

	UFUNCTION(BlueprintCallable, Category="Lobby")
	const TArray<FLobbySlotData>& GetAllSlots() const { return Slots; }

	UFUNCTION(BlueprintCallable, Category="Lobby")
	ULobbyDeviceRegistry* GetDeviceRegistry() const { return DeviceRegistry; }

	// Mutations (broadcast changes)
	UFUNCTION(BlueprintCallable, Category="Lobby")
	bool SetSlotControl(int32 SlotIndex, const FLobbyControlAssignment& NewControl);

	// RB/LB should call this (cycles through PresetLibrary's available preset IDs)
	UFUNCTION(BlueprintCallable, Category="Lobby|Presets")
	void CyclePreset(int32 SlotIndex, int32 Delta);

	UFUNCTION(BlueprintCallable, Category="Lobby|Presets")
	void SetSelectedPresetId(int32 SlotIndex, const FGuid& PresetId);

	UFUNCTION(BlueprintCallable, Category="Lobby|Control")
	void CycleSlotControl(int32 SlotIndex, int32 Delta);

	// Assign a physical device by explicit device id (keyboard/gamepad index) to a slot.
	UFUNCTION(BlueprintCallable, Category="Lobby|Control")
	bool AssignPhysicalDeviceToSlot(int32 SlotIndex, const FLobbyDeviceId& DeviceId);

	// Convenience: assign the physical device that corresponds to the given PlayerController.
	UFUNCTION(BlueprintCallable, Category="Lobby|Control")
	bool AssignPhysicalDeviceFromController(ALobbyPlayerController* RequestingPC, int32 SlotIndex);


protected:
	// Developer default preset packs (DataAssets) -> fed into PresetLibrarySubsystem on Init.
	UPROPERTY(EditDefaultsOnly, Category="Lobby|Presets")
	TArray<TObjectPtr<UPresetPackDataAsset>> DefaultDevPresetPacks;

private:
	UPROPERTY()
	TArray<FLobbySlotData> Slots;

	UPROPERTY()
	TObjectPtr<ULobbyDeviceRegistry> DeviceRegistry;

private:
	bool IsValidSlotIndex(int32 SlotIndex) const;
	void BroadcastSlotChanged(int32 SlotIndex);
	void InitializeDefaultSlots();
	
	void HandleDevicesChanged();
	FLobbyControlAssignment PickFallbackAssignment() const;

	// Ensures slot has a valid preset id (or assigns first available, or invalid guid if none)
	void EnsureValidPresetForSlot(int32 SlotIndex);
};
