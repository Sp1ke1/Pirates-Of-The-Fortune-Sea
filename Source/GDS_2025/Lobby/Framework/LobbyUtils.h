#pragma once

#include "CoreMinimal.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "GDS_2025/Lobby/Devices/LobbyDeviceRegistry.h"
#include "GDS_2025/Lobby/Data/LobbyTypes.h"

// Helper to infer a FLobbyDeviceId for a given PlayerController using the device registry.
static inline FLobbyDeviceId GetDeviceIdForController(APlayerController* PC, ULobbyDeviceRegistry* Registry)
{
    if (!PC)
    {
        return FLobbyDeviceId::Keyboard();
    }

    if (!Registry)
    {
        return FLobbyDeviceId::Keyboard();
    }

    if (ULocalPlayer* LP = PC->GetLocalPlayer())
    {
        const int32 ControllerId = LP->GetControllerId();
        if (ControllerId >= 0 && Registry->GetConnectedGamepadIndices().Contains(ControllerId))
        {
            return FLobbyDeviceId::Gamepad(ControllerId);
        }
    }

    return FLobbyDeviceId::Keyboard();
}
