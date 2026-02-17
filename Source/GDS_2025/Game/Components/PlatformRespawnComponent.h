#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlatformRespawnComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GDS_2025_API UPlatformRespawnComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPlatformRespawnComponent();

    // List of transforms where the player can be respawned. Editable in editor/Blueprints.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn")
    TArray<FTransform> RespawnPoints;

    // Respawn the given pawn to the transform at Index. Returns true if successful.
    UFUNCTION(BlueprintCallable, Category = "Respawn")
    bool RespawnPlayer(APawn* Pawn, int32 Index = 0);

    // Convenience: get number of respawn points
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Respawn")
    int32 GetRespawnPointCount() const { return RespawnPoints.Num(); }
};

