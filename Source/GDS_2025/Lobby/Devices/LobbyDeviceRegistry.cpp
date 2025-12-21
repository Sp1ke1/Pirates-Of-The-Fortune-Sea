#include "GDS_2025/Lobby/Devices/LobbyDeviceRegistry.h"

void ULobbyDeviceRegistry::Refresh()
{
	// Stub:
	// Later you can detect actual connected controllers, platform user mapping, etc.
	// For now we just keep MaxGamepadsToOffer and reservations.
}

bool ULobbyDeviceRegistry::IsReservableDeviceType(const ELobbyDeviceType Type)
{
	return Type == ELobbyDeviceType::Keyboard || Type == ELobbyDeviceType::Gamepad;
}

bool ULobbyDeviceRegistry::IsDeviceReserved(const FLobbyDeviceId& DeviceId) const
{
	return ReservedDevices.Contains(DeviceId);
}

bool ULobbyDeviceRegistry::IsDeviceReservedByOtherSlot(const FLobbyDeviceId& DeviceId, const int32 SlotIndex) const
{
	const int32* ExistingSlot = ReservedDevices.Find(DeviceId);
	return ExistingSlot && (*ExistingSlot != SlotIndex);
}

bool ULobbyDeviceRegistry::ReserveDeviceForSlot(const FLobbyDeviceId& DeviceId, const int32 SlotIndex)
{
	if (!IsReservableDeviceType(DeviceId.Type))
	{
		return true; // Nothing to reserve (None)
	}

	if (IsDeviceReservedByOtherSlot(DeviceId, SlotIndex))
	{
		return false;
	}

	ReservedDevices.Add(DeviceId, SlotIndex);
	return true;
}

void ULobbyDeviceRegistry::ReleaseSlot(const int32 SlotIndex)
{
	// Remove all reservations belonging to this slot
	TArray<FLobbyDeviceId> ToRemove;
	ToRemove.Reserve(ReservedDevices.Num());

	for (const auto& Pair : ReservedDevices)
	{
		if (Pair.Value == SlotIndex)
		{
			ToRemove.Add(Pair.Key);
		}
	}

	for (const FLobbyDeviceId& Id : ToRemove)
	{
		ReservedDevices.Remove(Id);
	}
}

TArray<FLobbyControlOption> ULobbyDeviceRegistry::GetAvailableOptions(const int32 SlotIndex) const
{
	TArray<FLobbyControlOption> Options;

	// Always allow these
	Options.Add(FLobbyControlOption(FText::FromString(TEXT("None")), FLobbyControlAssignment::None()));
	Options.Add(FLobbyControlOption(FText::FromString(TEXT("AI")), FLobbyControlAssignment::AI()));
	Options.Add(FLobbyControlOption(FText::FromString(TEXT("Matchmaking")), FLobbyControlAssignment::Matchmaking()));

	// Keyboard (single)
	{
		const FLobbyDeviceId KeyboardId = FLobbyDeviceId::Keyboard();
		if (!IsDeviceReservedByOtherSlot(KeyboardId, SlotIndex))
		{
			Options.Add(FLobbyControlOption(FText::FromString(TEXT("Keyboard")), FLobbyControlAssignment::Keyboard()));
		}
	}

	// Gamepads (0..MaxGamepadsToOffer-1)
	for (int32 Pad = 0; Pad < MaxGamepadsToOffer; ++Pad)
	{
		const FLobbyDeviceId PadId = FLobbyDeviceId::Gamepad(Pad);
		if (!IsDeviceReservedByOtherSlot(PadId, SlotIndex))
		{
			const FText Label = FText::FromString(FString::Printf(TEXT("Gamepad %d"), Pad + 1));
			Options.Add(FLobbyControlOption(Label, FLobbyControlAssignment::Gamepad(Pad)));
		}
	}

	return Options;
}
