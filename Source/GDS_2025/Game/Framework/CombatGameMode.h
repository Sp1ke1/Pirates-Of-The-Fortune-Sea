#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "CombatGameMode.generated.h"

UCLASS()
class GDS_2025_API ACombatGameMode : public AGameModeBase 
{
  GENERATED_BODY()

public:
  ACombatGameMode();

  UFUNCTION(BlueprintCallable, Category = "Pirates|Multiplayer")
  void AssignPiratesToLobbySlots(const TArray<class APawn *> &MapPirates);
};
