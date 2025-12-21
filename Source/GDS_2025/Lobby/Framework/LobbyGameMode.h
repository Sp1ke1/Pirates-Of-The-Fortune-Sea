#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "LobbyGameMode.generated.h"

class ULobbyGameInstance;
class ALobbySlotActor;
class ALobbyPlayerController;
struct FLobbySlotData;

UCLASS()
class ALobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ALobbyGameMode();

	virtual void BeginPlay() override;

	// Called by PlayerController when user presses A on the focused slot
	UFUNCTION(BlueprintCallable, Category="Lobby|UI")
	void OpenAssignControlUI(ALobbyPlayerController* RequestingPC, int32 SlotIndex);

	// Re-apply one slot to the world (slot actors, etc.)
	UFUNCTION(BlueprintCallable, Category="Lobby")
	void ApplySlotDataToWorld(int32 SlotIndex);

    UFUNCTION(BlueprintCallable, Category="Lobby")
    ALobbySlotActor* GetSlotActor(int32 SlotIndex) const
    {
        return SlotActors.IsValidIndex(SlotIndex) ? SlotActors[SlotIndex].Get() : nullptr;
    }
protected:
	// Find slot actors placed in the level and sort them by SlotIndex
	UFUNCTION(BlueprintCallable, Category="Lobby")
	void CollectSlotActors();

	// BP hook: create and show the widget however you want
	UFUNCTION(BlueprintImplementableEvent, Category="Lobby|UI")
	void BP_OpenAssignControlUI(ALobbyPlayerController* RequestingPC, int32 SlotIndex);

private:
	// Cached pointer to our GameInstance
	UPROPERTY()
	TObjectPtr<ULobbyGameInstance> LobbyGI;

	// Slot actors in the scene, expected 4
	UPROPERTY()
	TArray<TObjectPtr<ALobbySlotActor>> SlotActors;

private:
	void HandleSlotChanged(int32 SlotIndex, const FLobbySlotData& NewData);
};
