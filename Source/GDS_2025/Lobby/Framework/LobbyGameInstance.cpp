#include "GDS_2025/Lobby/Framework/LobbyGameInstance.h"

#include "GDS_2025/Lobby/Devices/LobbyDeviceRegistry.h"
#include "GDS_2025/Lobby/Presets/PresetLibrarySubsystem.h"
#include "GDS_2025/Lobby/Presets/PresetPackDataAsset.h"
#include "GDS_2025/Lobby/Framework/LobbyPlayerController.h"
#include "Engine/LocalPlayer.h"
#include "InputCoreTypes.h"

void ULobbyGameInstance::Init()
{
	Super::Init();

	// Devices
	DeviceRegistry = NewObject<ULobbyDeviceRegistry>(this, ULobbyDeviceRegistry::StaticClass());
	if (DeviceRegistry)
	{
		DeviceRegistry->Refresh();
	}
	if (DeviceRegistry)
	{
		DeviceRegistry->OnDevicesChangedNative.AddUObject(this, &ULobbyGameInstance::HandleDevicesChanged);
	}

	// Presets: feed dev packs into subsystem
	if (UPresetLibrarySubsystem* PresetLib = GetSubsystem<UPresetLibrarySubsystem>())
	{
		PresetLib->SetDevPresetPacks(DefaultDevPresetPacks);
		// Trigger preload now that dev packs are provided so meshes start loading during Init.
		PresetLib->PreloadAllPresetMeshes();
	}

	InitializeDefaultSlots();
}

bool ULobbyGameInstance::IsValidSlotIndex(const int32 SlotIndex) const
{
	return SlotIndex >= 0 && SlotIndex < Slots.Num();
}

const FLobbySlotData& ULobbyGameInstance::GetSlotData(const int32 SlotIndex) const
{
	static FLobbySlotData Dummy;
	return IsValidSlotIndex(SlotIndex) ? Slots[SlotIndex] : Dummy;
}

void ULobbyGameInstance::BroadcastSlotChanged(const int32 SlotIndex)
{
	if (IsValidSlotIndex(SlotIndex))
	{
		OnLobbySlotChangedNative.Broadcast(SlotIndex, Slots[SlotIndex]);
	}
}

void ULobbyGameInstance::InitializeDefaultSlots()
{
	Slots.SetNum(NumLobbySlots);

	// Example colors (tweak anytime)
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
		Slots[i].PlayerColor = Colors[i];
		Slots[i].bReady = false;
		Slots[i].SelectedPresetId = FGuid(); // will be fixed by EnsureValidPresetForSlot

		EnsureValidPresetForSlot(i);
		BroadcastSlotChanged(i);
	}
}

void ULobbyGameInstance::EnsureValidPresetForSlot(const int32 SlotIndex)
{
	if (!IsValidSlotIndex(SlotIndex))
	{
		return;
	}

	UPresetLibrarySubsystem* PresetLib = GetSubsystem<UPresetLibrarySubsystem>();
	if (!PresetLib)
	{
		Slots[SlotIndex].SelectedPresetId = FGuid();
		return;
	}

	const TArray<FGuid> Ids = PresetLib->GetAllPresetIds();
	if (Ids.Num() <= 0)
	{
		Slots[SlotIndex].SelectedPresetId = FGuid();
		return;
	}

	// If current id is invalid or unknown, set first available
	const FGuid Current = Slots[SlotIndex].SelectedPresetId;
	const bool bHasCurrent = Current.IsValid() && (Ids.IndexOfByKey(Current) != INDEX_NONE);

	if (!bHasCurrent)
	{
		Slots[SlotIndex].SelectedPresetId = Ids[0];
	}
}

bool ULobbyGameInstance::SetSlotControl(const int32 SlotIndex, const FLobbyControlAssignment& NewControl)
{
	if (!IsValidSlotIndex(SlotIndex) || !DeviceRegistry)
	{
		return false;
	}

	// Release old reservation (if any)
	DeviceRegistry->ReleaseSlot(SlotIndex);

	// Reserve new one if needed
	const bool bReservedOk = DeviceRegistry->ReserveDeviceForSlot(NewControl.DeviceId, SlotIndex);
	if (!bReservedOk)
	{
		Slots[SlotIndex].Control = FLobbyControlAssignment::None();
		BroadcastSlotChanged(SlotIndex);
		return false;
	}

	Slots[SlotIndex].Control = NewControl;
	BroadcastSlotChanged(SlotIndex);
	return true;
}

void ULobbyGameInstance::SetSelectedPresetId(const int32 SlotIndex, const FGuid& PresetId)
{
	if (!IsValidSlotIndex(SlotIndex))
	{
		return;
	}

	Slots[SlotIndex].SelectedPresetId = PresetId;

	// Make sure it's valid; if not, snap to first available (or invalid if none)
	EnsureValidPresetForSlot(SlotIndex);

	BroadcastSlotChanged(SlotIndex);
}

// Preset cycling moved to ULobbySubsystem (LobbySubsystem.*)

void ULobbyGameInstance::CycleSlotControl(const int32 SlotIndex, const int32 Delta)
{
	if (!IsValidSlotIndex(SlotIndex) || !DeviceRegistry)
	{
		return;
	}

	// Cycle only virtual options: None, AI (Bot), Matchmaking (Online).
	TArray<FLobbyControlAssignment> Options;
	Options.Add(FLobbyControlAssignment::None());
	Options.Add(FLobbyControlAssignment::AI());
	Options.Add(FLobbyControlAssignment::Matchmaking());

	const FLobbyControlAssignment Current = Slots[SlotIndex].Control;

	int32 CurrentIndex = Options.IndexOfByPredicate([&](const FLobbyControlAssignment& A)
	{
		return A.Source == Current.Source && A.DeviceId == Current.DeviceId;
	});

	if (CurrentIndex == INDEX_NONE)
	{
		CurrentIndex = 0;
	}

	int32 NextIndex = (CurrentIndex + Delta) % Options.Num();
	if (NextIndex < 0) NextIndex += Options.Num();

	SetSlotControl(SlotIndex, Options[NextIndex]);
}

bool ULobbyGameInstance::AssignPhysicalDeviceToSlot(const int32 SlotIndex, const FLobbyDeviceId& DeviceId)
{
	if (!IsValidSlotIndex(SlotIndex) || !DeviceRegistry)
	{
		return false;
	}

	// If device already reserved by another slot, move that slot to Matchmaking (online) first.
	const int32 ExistingSlot = DeviceRegistry->GetReservedSlotForDevice(DeviceId);
	if (ExistingSlot != INDEX_NONE && ExistingSlot != SlotIndex)
	{
		SetSlotControl(ExistingSlot, FLobbyControlAssignment::Matchmaking());
	}

	// Apply new physical assignment
	FLobbyControlAssignment NewA;
	if (DeviceId.Type == ELobbyDeviceType::Keyboard)
	{
		NewA = FLobbyControlAssignment::Keyboard();
	}
	else if (DeviceId.Type == ELobbyDeviceType::Gamepad)
	{
		NewA = FLobbyControlAssignment::Gamepad(DeviceId.Index);
	}
	else
	{
		return false;
	}

	return SetSlotControl(SlotIndex, NewA);
}

bool ULobbyGameInstance::AssignPhysicalDeviceFromController(ALobbyPlayerController* RequestingPC, const int32 SlotIndex)
{
	if (!RequestingPC || !DeviceRegistry)
	{
		return false;
	}

	// Heuristic: check which type of input is currently pressed on this controller.
	// Prefer gamepad if a common gamepad confirm button is down; otherwise prefer keyboard.
	bool bGamepadPressed = false;
	bool bKeyboardPressed = false;

	// Common keys to check
	const TArray<FKey> GamepadConfirmKeys = {
		EKeys::Gamepad_FaceButton_Bottom,
		EKeys::Gamepad_FaceButton_Right,
		EKeys::Gamepad_FaceButton_Left,
		EKeys::Gamepad_FaceButton_Top,
		EKeys::Gamepad_Special_Left,
		EKeys::Gamepad_Special_Right
	};

	const TArray<FKey> KeyboardConfirmKeys = {
		EKeys::Enter,
		EKeys::SpaceBar
	};

	for (const FKey& K : GamepadConfirmKeys)
	{
		if (RequestingPC->IsInputKeyDown(K))
		{
			bGamepadPressed = true;
			break;
		}
	}

	if (!bGamepadPressed)
	{
		for (const FKey& K : KeyboardConfirmKeys)
		{
			if (RequestingPC->IsInputKeyDown(K))
			{
				bKeyboardPressed = true;
				break;
			}
		}
	}

	FLobbyDeviceId DevId = FLobbyDeviceId::None();
	if (ULocalPlayer* LP = RequestingPC->GetLocalPlayer())
	{
		const int32 ControllerId = LP->GetControllerId();

		if (bGamepadPressed && ControllerId >= 0 && DeviceRegistry->GetConnectedGamepadIndices().Contains(ControllerId))
		{
			DevId = FLobbyDeviceId::Gamepad(ControllerId);
		}
		else if (bKeyboardPressed)
		{
			DevId = FLobbyDeviceId::Keyboard();
		}
		else
		{
			// If we couldn't detect a button but controller id looks like a gamepad, prefer keyboard only if keyboard input is available.
			if (ControllerId >= 0 && DeviceRegistry->GetConnectedGamepadIndices().Contains(ControllerId) && !RequestingPC->IsInputKeyDown(EKeys::Enter) )
			{
				DevId = FLobbyDeviceId::Gamepad(ControllerId);
			}
			else
			{
				DevId = FLobbyDeviceId::Keyboard();
			}
		}
	}

	if (DevId.Type == ELobbyDeviceType::None)
	{
		return false;
	}

	return AssignPhysicalDeviceToSlot(SlotIndex, DevId);
}

void ULobbyGameInstance::HandleDevicesChanged()
{
	if (!DeviceRegistry)
	{
		return;
	}

	for (int32 i = 0; i < Slots.Num(); ++i)
	{
		if (!DeviceRegistry->IsAssignmentValid(Slots[i].Control))
		{
			TArray<FLobbyControlAssignment> Options;
			DeviceRegistry->BuildAssignableOptions(Options);

			const FLobbyControlAssignment Fallback = (Options.Num() > 0)
				? Options[0]
				: FLobbyControlAssignment::None();

			SetSlotControl(i, Fallback);
		}
		else
		{
			BroadcastSlotChanged(i);
		}
	}
}
