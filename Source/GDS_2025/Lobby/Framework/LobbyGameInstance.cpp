#include "GDS_2025/Lobby/Framework/LobbyGameInstance.h"

#include "GDS_2025/Lobby/Devices/LobbyDeviceRegistry.h"
#include "GDS_2025/Lobby/Presets/PresetLibrarySubsystem.h"
#include "GDS_2025/Lobby/Presets/PresetPackDataAsset.h"

void ULobbyGameInstance::Init()
{
	Super::Init();

	// Devices
	DeviceRegistry = NewObject<ULobbyDeviceRegistry>(this, ULobbyDeviceRegistry::StaticClass());
	if (DeviceRegistry)
	{
		DeviceRegistry->Refresh();
	}

	// Presets: feed dev packs into subsystem
	if (UPresetLibrarySubsystem* PresetLib = GetSubsystem<UPresetLibrarySubsystem>())
	{
		PresetLib->SetDevPresetPacks(DefaultDevPresetPacks);
		// Subsystem loads user presets in Initialize(), but you can force here if you want:
		// PresetLib->LoadUserPresets();
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

void ULobbyGameInstance::CyclePreset(const int32 SlotIndex, const int32 Delta)
{
	if (!IsValidSlotIndex(SlotIndex))
	{
		return;
	}

	UPresetLibrarySubsystem* PresetLib = GetSubsystem<UPresetLibrarySubsystem>();
	if (!PresetLib)
	{
		return;
	}

	const TArray<FGuid> Ids = PresetLib->GetAllPresetIds();
	if (Ids.Num() <= 0)
	{
		return;
	}

	// Ensure current is valid before cycling
	EnsureValidPresetForSlot(SlotIndex);

	const FGuid Current = Slots[SlotIndex].SelectedPresetId;

	int32 CurrentIndex = Ids.IndexOfByKey(Current);
	if (CurrentIndex == INDEX_NONE)
	{
		CurrentIndex = 0;
	}

	int32 NextIndex = (CurrentIndex + Delta) % Ids.Num();
	if (NextIndex < 0)
	{
		NextIndex += Ids.Num();
	}

	Slots[SlotIndex].SelectedPresetId = Ids[NextIndex];
	BroadcastSlotChanged(SlotIndex);
}
