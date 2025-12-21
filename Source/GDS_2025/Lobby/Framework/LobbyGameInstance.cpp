#include "GDS_2025/Lobby/Framework/LobbyGameInstance.h"
#include "GDS_2025/Lobby/Devices/LobbyDeviceRegistry.h"

void ULobbyGameInstance::Init()
{
	Super::Init();

	DeviceRegistry = NewObject<ULobbyDeviceRegistry>(this, ULobbyDeviceRegistry::StaticClass());
	if (DeviceRegistry)
	{
		DeviceRegistry->Refresh();
	}

	InitializeDefaultSlots();
}

bool ULobbyGameInstance::IsValidSlotIndex(const int32 SlotIndex) const
{
	return SlotIndex >= 0 && SlotIndex < Slots.Num();
}

void ULobbyGameInstance::InitializeDefaultSlots()
{
	Slots.SetNum(NumLobbySlots);

	// Example colors, can be anything you want
	const FLinearColor Colors[NumLobbySlots] =
	{
		FLinearColor(0.95f, 0.25f, 0.25f, 1.0f),
		FLinearColor(0.25f, 0.55f, 0.95f, 1.0f),
		FLinearColor(0.25f, 0.90f, 0.35f, 1.0f),
		FLinearColor(0.90f, 0.85f, 0.25f, 1.0f)
	};

	for (int32 i = 0; i < NumLobbySlots; ++i)
	{
		Slots[i].SlotIndex = i;
		Slots[i].Control = FLobbyControlAssignment::None();
		Slots[i].SkinIndex = 0;
		Slots[i].PlayerColor = Colors[i];
		Slots[i].bReady = false;

		BroadcastSlotChanged(i);
	}
}

const FLobbySlotData& ULobbyGameInstance::GetSlotData(const int32 SlotIndex) const
{
	static FLobbySlotData Dummy;

	if (!IsValidSlotIndex(SlotIndex))
	{
		return Dummy;
	}

	return Slots[SlotIndex];
}

void ULobbyGameInstance::BroadcastSlotChanged(const int32 SlotIndex)
{
	if (!IsValidSlotIndex(SlotIndex))
	{
		return;
	}

	OnLobbySlotChangedNative.Broadcast(SlotIndex, Slots[SlotIndex]);
}

bool ULobbyGameInstance::SetSlotControl(const int32 SlotIndex, const FLobbyControlAssignment& NewControl)
{
	if (!IsValidSlotIndex(SlotIndex) || !DeviceRegistry)
	{
		return false;
	}

	// Release old reservation (if any)
	DeviceRegistry->ReleaseSlot(SlotIndex);

	// Try reserve new one (if needed)
	const FLobbyDeviceId NewDevice = NewControl.DeviceId;
	const bool bReservedOk = DeviceRegistry->ReserveDeviceForSlot(NewDevice, SlotIndex);
	if (!bReservedOk)
	{
		// Reservation failed -> keep slot empty (or you can restore previous state if you prefer)
		Slots[SlotIndex].Control = FLobbyControlAssignment::None();
		BroadcastSlotChanged(SlotIndex);
		return false;
	}

	Slots[SlotIndex].Control = NewControl;
	BroadcastSlotChanged(SlotIndex);
	return true;
}

void ULobbyGameInstance::CycleSkin(const int32 SlotIndex, const int32 Delta)
{
	if (!IsValidSlotIndex(SlotIndex))
	{
		return;
	}

	// Simple wrap-around if you want; for now keep non-negative
	int32 NewIndex = Slots[SlotIndex].SkinIndex + Delta;
	if (NewIndex < 0)
	{
		NewIndex = 0;
	}

	Slots[SlotIndex].SkinIndex = NewIndex;
	BroadcastSlotChanged(SlotIndex);
}

void ULobbyGameInstance::SetSkinIndex(const int32 SlotIndex, const int32 NewSkinIndex)
{
	if (!IsValidSlotIndex(SlotIndex))
	{
		return;
	}

	Slots[SlotIndex].SkinIndex = FMath::Max(0, NewSkinIndex);
	BroadcastSlotChanged(SlotIndex);
}
