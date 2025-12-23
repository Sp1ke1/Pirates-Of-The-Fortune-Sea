// Copyright: project
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GDS_2025/Lobby/Data/LobbySlotData.h"
#include "LobbySubsystem.generated.h"

class ULobbyGameInstance;
class UPresetLibrarySubsystem;

UCLASS()
class ULobbySubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // Cycle presets (legacy name kept) — internal calls allowed
    void CyclePreset(int32 SlotIndex, int32 Delta);

    // Cycle preset with requester device to enforce permission rules
    void CyclePresetWithDevice(int32 SlotIndex, int32 Delta, const FLobbyDeviceId& RequestingDevice);

protected:
    // Cached pointer to GameInstance for slot data access
    ULobbyGameInstance* LobbyGI = nullptr;
};
