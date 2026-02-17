#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ObjectSpawner.generated.h"

class UBoxComponent;

UCLASS(Blueprintable)
class GDS_2025_API AObjectSpawner : public AActor
{
    GENERATED_BODY()

public:
    AObjectSpawner();

    // Bounding box used as spawn area
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawner")
    UBoxComponent* SpawnBounds;

    // List of actor classes that can be spawned by this spawner
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
    TArray<TSubclassOf<AActor>> SpawnableClasses;

    // If true, starts the spawn timer at BeginPlay
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
    bool bSpawnOnBeginPlay = false;

    // Spawn a single random actor from the SpawnableClasses list at a random point inside the box
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Spawner")
    void SpawnRandom();

    // How often to spawn (seconds). When spawning is active, spawns one actor every SpawnRate seconds.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.01", UIMin = "0.01"), Category = "Spawner")
    float SpawnRate = 1.0f;

    // Start/stop timer from Blueprints
    UFUNCTION(BlueprintCallable, Category = "Spawner")
    void StartSpawningTimer();

    UFUNCTION(BlueprintCallable, Category = "Spawner")
    void StopSpawningTimer();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // Helper: returns a random point inside the SpawnBounds box (world space)
    FVector GetRandomPointInBounds() const;

    // Timer callback
    void SpawnTimerTick();

private:
    // Handle for the spawn timer
    FTimerHandle SpawnTimerHandle;
};
