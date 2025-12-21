#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GDS_2025/Lobby/Data/LobbySlotData.h"

#include "LobbyGameInstance.generated.h"

class ULobbyDeviceRegistry;

// SlotIndex, NewSlotData
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnLobbySlotChangedNative, int32, const FLobbySlotData&);

UCLASS()
class ULobbyGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	static constexpr int32 NumLobbySlots = 4;

	// Native event for C++ listeners (GameMode, etc.)
	FOnLobbySlotChangedNative OnLobbySlotChangedNative;

	// UObject lifecycle
	virtual void Init() override;

	// Data access
	UFUNCTION(BlueprintCallable, Category="Lobby")
	const FLobbySlotData& GetSlotData(int32 SlotIndex) const;

	UFUNCTION(BlueprintCallable, Category="Lobby")
	const TArray<FLobbySlotData>& GetAllSlots() const { return Slots; }

	UFUNCTION(BlueprintCallable, Category="Lobby")
	ULobbyDeviceRegistry* GetDeviceRegistry() const { return DeviceRegistry; }

	// Mutations (these broadcast changes)
	UFUNCTION(BlueprintCallable, Category="Lobby")
	bool SetSlotControl(int32 SlotIndex, const FLobbyControlAssignment& NewControl);

	UFUNCTION(BlueprintCallable, Category="Lobby")
	void CycleSkin(int32 SlotIndex, int32 Delta);

	UFUNCTION(BlueprintCallable, Category="Lobby")
	void SetSkinIndex(int32 SlotIndex, int32 NewSkinIndex);

private:
	UPROPERTY()
	TArray<FLobbySlotData> Slots;

	UPROPERTY()
	TObjectPtr<ULobbyDeviceRegistry> DeviceRegistry;

private:
	bool IsValidSlotIndex(int32 SlotIndex) const;
	void BroadcastSlotChanged(int32 SlotIndex);
	void InitializeDefaultSlots();
};
