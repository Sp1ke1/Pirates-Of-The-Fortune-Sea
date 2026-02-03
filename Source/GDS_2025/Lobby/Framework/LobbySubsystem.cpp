#include "GDS_2025/Lobby/Framework/LobbySubsystem.h"

#include "GDS_2025/Lobby/Framework/LobbyGameInstance.h"
#include "GDS_2025/Customization/Presets/PresetLibrarySubsystem.h"

void ULobbySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    LobbyGI = Cast<ULobbyGameInstance>(GetGameInstance());
    if (!LobbyGI)
    {
        UE_LOG(LogTemp, Warning, TEXT("[LobbySubsystem] GameInstance is not ULobbyGameInstance."));
    }
}

void ULobbySubsystem::CyclePreset(int32 SlotIndex, int32 Delta)
{
    CyclePresetWithDevice(SlotIndex, Delta, FLobbyDeviceId::None());
}

void ULobbySubsystem::CyclePresetWithDevice(int32 SlotIndex, int32 Delta, const FLobbyDeviceId& RequestingDevice)
{
    if (!LobbyGI)
    {
        return;
    }

    const TArray<FLobbySlotData>& All = LobbyGI->GetAllSlots();
    if (!All.IsValidIndex(SlotIndex))
    {
        return;
    }

    const FLobbySlotData& Slot = LobbyGI->GetSlotData(SlotIndex);

    // Permission check — same rules as previous implementation
    const FLobbyControlAssignment& CurrentControl = Slot.Control;
    bool bAllowed = false;

    if (RequestingDevice.Type == ELobbyDeviceType::None)
    {
        bAllowed = true;
    }
    else
    {
        switch (CurrentControl.Source)
        {
        case ELobbyControlSource::None:
        case ELobbyControlSource::AI:
        case ELobbyControlSource::Matchmaking:
            bAllowed = true;
            break;
        case ELobbyControlSource::Keyboard:
            bAllowed = (RequestingDevice.Type == ELobbyDeviceType::Keyboard);
            break;
        case ELobbyControlSource::Gamepad:
            bAllowed = (RequestingDevice.Type == ELobbyDeviceType::Gamepad && CurrentControl.DeviceId == RequestingDevice);
            break;
        default:
            bAllowed = false;
            break;
        }
    }

    if (!bAllowed)
    {
        return;
    }

    UPresetLibrarySubsystem* PresetLib = GetGameInstance()->GetSubsystem<UPresetLibrarySubsystem>();
    if (!PresetLib)
    {
        return;
    }

    const TArray<FGuid> Ids = PresetLib->GetAllPresetIds();
    if (Ids.Num() <= 0)
    {
        return;
    }

    // Determine current index
    const FGuid Current = Slot.SelectedPresetId;
    int32 CurrentIndex = Ids.IndexOfByKey(Current);
    if (CurrentIndex == INDEX_NONE)
    {
        CurrentIndex = 0;
    }

    int32 NextIndex = (CurrentIndex + Delta) % Ids.Num();
    if (NextIndex < 0) NextIndex += Ids.Num();

    // Apply via GameInstance setter so broadcasts remain centralized
    LobbyGI->SetSelectedPresetId(SlotIndex, Ids[NextIndex]);
}
