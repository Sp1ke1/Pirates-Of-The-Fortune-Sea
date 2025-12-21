#include "GDS_2025/Lobby/Framework/LobbyGameMode.h"

#include "Engine/World.h"
#include "EngineUtils.h"

#include "GDS_2025/Lobby/Framework/LobbyGameInstance.h"
#include "GDS_2025/Lobby/Data/LobbySlotData.h"
#include "GDS_2025/Lobby/Actors/LobbySlotActor.h"

ALobbyGameMode::ALobbyGameMode()
{
}

void ALobbyGameMode::BeginPlay()
{
	Super::BeginPlay();

	LobbyGI = Cast<ULobbyGameInstance>(GetGameInstance());
	if (!LobbyGI)
	{
		UE_LOG(LogTemp, Error, TEXT("[LobbyGameMode] GameInstance is not ULobbyGameInstance. Check project settings."));
		return;
	}

	CollectSlotActors();

	LobbyGI->OnLobbySlotChangedNative.AddUObject(this, &ALobbyGameMode::HandleSlotChanged);

	for (int32 i = 0; i < ULobbyGameInstance::NumLobbySlots; ++i)
	{
		ApplySlotDataToWorld(i);
	}
}

void ALobbyGameMode::CollectSlotActors()
{
	SlotActors.Reset();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<ALobbySlotActor> It(World); It; ++It)
	{
		SlotActors.Add(*It);
	}

	SlotActors.Sort([](const ALobbySlotActor& A, const ALobbySlotActor& B)
	{
		return A.SlotIndex < B.SlotIndex;
	});

	UE_LOG(LogTemp, Log, TEXT("[LobbyGameMode] Collected %d LobbySlotActors"), SlotActors.Num());
}

void ALobbyGameMode::HandleSlotChanged(const int32 SlotIndex, const FLobbySlotData& NewData)
{
	ApplySlotDataToWorld(SlotIndex);
}

void ALobbyGameMode::ApplySlotDataToWorld(const int32 SlotIndex)
{
	if (!LobbyGI)
	{
		return;
	}

	const FLobbySlotData& Data = LobbyGI->GetSlotData(SlotIndex);

	if (SlotActors.IsValidIndex(SlotIndex) && SlotActors[SlotIndex])
	{
		SlotActors[SlotIndex]->ApplyData(Data);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[LobbyGameMode] No SlotActor for slot %d. Place 4 actors and set SlotIndex 0..3."), SlotIndex);
	}
}

void ALobbyGameMode::OpenAssignControlUI(ALobbyPlayerController* RequestingPC, const int32 SlotIndex)
{
	BP_OpenAssignControlUI(RequestingPC, SlotIndex);
}
