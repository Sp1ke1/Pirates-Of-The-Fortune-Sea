#include "GDS_2025/Lobby/Devices/LobbyDeviceRegistry.h"

#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"

void ULobbyDeviceRegistry::Refresh()
{
	TArray<int32> NewGamepads;
	QueryConnectedGamepads(NewGamepads);

	NewGamepads.Sort();

	const bool bChanged = (NewGamepads != ConnectedGamepadIndices);
	ConnectedGamepadIndices = MoveTemp(NewGamepads);

	// Drop reservations for devices that no longer exist
	{
		TArray<FLobbyDeviceId> ToRemove;

		for (const TPair<FLobbyDeviceId, int32>& Pair : ReservedByDeviceId)
		{
			const FLobbyDeviceId& DevId = Pair.Key;

			bool bStillValid = true;
			switch (DevId.Type)
			{
			case ELobbyDeviceType::Keyboard:
				bStillValid = bKeyboardAvailable;
				break;

			case ELobbyDeviceType::Gamepad:
				bStillValid = IsGamepadConnectedIndex(DevId.Index);
				break;

			case ELobbyDeviceType::None:
			default:
				bStillValid = true;
				break;
			}

			if (!bStillValid)
			{
				ToRemove.Add(DevId);
			}
		}

		for (const FLobbyDeviceId& DevId : ToRemove)
		{
			if (const int32* SlotPtr = ReservedByDeviceId.Find(DevId))
			{
				ReservedBySlot.Remove(*SlotPtr);
			}
			ReservedByDeviceId.Remove(DevId);
		}
	}

	if (bChanged)
	{
		OnDevicesChangedNative.Broadcast();
	}
}

void ULobbyDeviceRegistry::QueryConnectedGamepads(TArray<int32>& OutGamepadIndices) const
{
	OutGamepadIndices.Reset();
	for (int32 i = 0; i < MaxSupportedGamepads; ++i)
	{
		OutGamepadIndices.Add(i);
	}
}

bool ULobbyDeviceRegistry::IsGamepadConnectedIndex(const int32 GamepadIndex) const
{
	return ConnectedGamepadIndices.Contains(GamepadIndex);
}

bool ULobbyDeviceRegistry::IsAssignmentValid(const FLobbyControlAssignment& A) const
{
	switch (A.Source)
	{
	case ELobbyControlSource::None:
		return true;

	case ELobbyControlSource::AI:
		return true;

	case ELobbyControlSource::Keyboard:
		return bKeyboardAvailable;

	case ELobbyControlSource::Gamepad:
		return IsGamepadConnectedIndex(A.DeviceId.Index);

	case ELobbyControlSource::Matchmaking:
		// Matchmaking is not a physical device. Keep valid here.
		return true;

	default:
		return false;
	}
}

bool ULobbyDeviceRegistry::ReserveDeviceForSlot(const FLobbyDeviceId& DeviceId, const int32 SlotIndex)
{
	if (SlotIndex < 0)
	{
		return false;
	}

	// None means no physical reservation required.
	if (DeviceId.Type == ELobbyDeviceType::None)
	{
		return true;
	}

	// Release any current reservation for this slot.
	ReleaseSlot(SlotIndex);

	// Keyboard: single-owner reservation (common for local co-op).
	if (DeviceId.Type == ELobbyDeviceType::Keyboard)
	{
		if (!bKeyboardAvailable)
		{
			return false;
		}

		if (const int32* ExistingSlot = ReservedByDeviceId.Find(DeviceId))
		{
			return (*ExistingSlot == SlotIndex);
		}

		ReservedByDeviceId.Add(DeviceId, SlotIndex);
		ReservedBySlot.Add(SlotIndex, DeviceId);
		return true;
	}

	// Gamepad: must be connected.
	if (DeviceId.Type == ELobbyDeviceType::Gamepad)
	{
		if (!IsGamepadConnectedIndex(DeviceId.Index))
		{
			return false;
		}

		if (const int32* ExistingSlot = ReservedByDeviceId.Find(DeviceId))
		{
			return (*ExistingSlot == SlotIndex);
		}

		ReservedByDeviceId.Add(DeviceId, SlotIndex);
		ReservedBySlot.Add(SlotIndex, DeviceId);
		return true;
	}

	return false;
}

void ULobbyDeviceRegistry::ReleaseSlot(const int32 SlotIndex)
{
	if (const FLobbyDeviceId* DevIdPtr = ReservedBySlot.Find(SlotIndex))
	{
		const FLobbyDeviceId DevId = *DevIdPtr;
		ReservedBySlot.Remove(SlotIndex);
		ReservedByDeviceId.Remove(DevId);
	}
}

void ULobbyDeviceRegistry::BuildAssignableOptions(TArray<FLobbyControlAssignment>& OutOptions) const
{
	OutOptions.Reset();

	// 1) None is always available.
	OutOptions.Add(FLobbyControlAssignment::None());

	// 2) Keyboard (if available and not reserved).
	if (bKeyboardAvailable)
	{
		const FLobbyDeviceId KeyboardId = FLobbyDeviceId::Keyboard();
		if (!ReservedByDeviceId.Contains(KeyboardId))
		{
			FLobbyControlAssignment A;
			A.Source = ELobbyControlSource::Keyboard;
			A.DeviceId = KeyboardId;
			OutOptions.Add(A);
		}
	}

	// 3) Free connected gamepads.
	for (const int32 GpIndex : ConnectedGamepadIndices)
	{
		const FLobbyDeviceId GpId = FLobbyDeviceId::Gamepad(GpIndex);
		if (ReservedByDeviceId.Contains(GpId))
		{
			continue;
		}

		FLobbyControlAssignment A;
		A.Source = ELobbyControlSource::Gamepad;
		A.DeviceId = GpId;
		OutOptions.Add(A);
	}

	// 4) AI is always available.
	{
		FLobbyControlAssignment A;
		A.Source = ELobbyControlSource::AI;
		A.DeviceId = FLobbyDeviceId::None();
		OutOptions.Add(A);
	}
}

int32 ULobbyDeviceRegistry::GetGamepadDisplayNumber(const int32 GamepadIndex) const
{
	// Stable display numbering based on sorted connected indices.
	const int32 SortedIndex = ConnectedGamepadIndices.IndexOfByKey(GamepadIndex);
	return (SortedIndex == INDEX_NONE) ? 0 : (SortedIndex + 1);
}

FText ULobbyDeviceRegistry::ToDisplayText(const FLobbyControlAssignment& A) const
{
	switch (A.Source)
	{
	case ELobbyControlSource::None:
		return FText::FromString(TEXT("NONE"));

	case ELobbyControlSource::Keyboard:
		return FText::FromString(TEXT("KEYBOARD"));

	case ELobbyControlSource::AI:
		return FText::FromString(TEXT("BOT"));

	case ELobbyControlSource::Gamepad:
	{
		const int32 Num = GetGamepadDisplayNumber(A.DeviceId.Index);
		if (Num > 0)
		{
			return FText::FromString(FString::Printf(TEXT("GAMEPAD %d"), Num));
		}
		return FText::FromString(TEXT("GAMEPAD"));
	}

	case ELobbyControlSource::Matchmaking:
		return FText::FromString(TEXT("ONLINE"));

	default:
		return FText::FromString(TEXT("UNKNOWN"));
	}
}
