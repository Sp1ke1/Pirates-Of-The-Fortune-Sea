#include "CombatGameMode.h"
#include "Engine/World.h"
#include "GDS_2025/Game/Components/BotLogicComponent.h"
#include "GDS_2025/Lobby/Data/LobbySlotData.h"
#include "GDS_2025/Lobby/Data/LobbyTypes.h"
#include "GDS_2025/Lobby/Framework/LobbyGameInstance.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

ACombatGameMode::ACombatGameMode() {}

void ACombatGameMode::AssignPiratesToLobbySlots(const TArray<APawn*>& MapPirates)
{
	UWorld* World = GetWorld();
	if (!World) return;

	ULobbyGameInstance* GameInstance = Cast<ULobbyGameInstance>(GetGameInstance());
	if (!GameInstance) return;

	const TArray<FLobbySlotData>& Slots = GameInstance->GetAllSlots();

	int32 HumanPlayerCount = 0;

	for (int32 i = 0; i < MapPirates.Num(); ++i)
	{
		APawn* PiratePawn = MapPirates[i];
		if (!PiratePawn) continue;

		if (i >= Slots.Num())
		{
			PiratePawn->Destroy();
			continue;
		}

		const FLobbySlotData& Slot = Slots[i];

		if (Slot.Control.Source == ELobbyControlSource::Keyboard ||
			Slot.Control.Source == ELobbyControlSource::Gamepad)
		{
			APlayerController* PC = nullptr;

			if (HumanPlayerCount == 0)
			{
				PC = UGameplayStatics::GetPlayerController(World, 0);
			}
			else
			{
				PC = UGameplayStatics::CreatePlayer(World, -1, true);
			}

			if (PC)
			{
				PC->Possess(PiratePawn);
			}

			HumanPlayerCount++;
		}
		else if (Slot.Control.Source == ELobbyControlSource::AI)
		{
			UBotLogicComponent* BotLogicComponent = PiratePawn->GetComponentByClass<UBotLogicComponent>();
			if (BotLogicComponent)
			{
				BotLogicComponent->SetActive(true);
			}
		}
		else
		{
			PiratePawn->Destroy();
		}
	}
}
