#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Platform.generated.h"

class UPlatformMassBoundsComponent;
class UPlatformRotationComponent;
class USceneComponent;

UCLASS()
class GDS_2025_API APlatform : public AActor
{
    GENERATED_BODY()

public:
    APlatform();

    // Root scene component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform")
    USceneComponent* RootScene;

    // Mass bounds component that detects masses on the platform
    // You can add multiple MassBounds components to the actor (in C++ or in Blueprint).
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform")
    TArray<UPlatformMassBoundsComponent*> MassBoundsComponents;

    // Rotation component that rotates the platform according to masses
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform")
    UPlatformRotationComponent* RotationComponent;

    // Spawn transforms for players. Editable in editor/Blueprint to specify where players should respawn on this platform.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform")
    TArray<FTransform> PlayerSpawnTransforms;

protected:
    virtual void BeginPlay() override;

public:
    // Helper to get all mass bounds (gathers dynamically)
    UFUNCTION(BlueprintCallable, Category = "Platform")
    const TArray<UPlatformMassBoundsComponent*>& GetMassBoundsComponents() const { return MassBoundsComponents; }

    // Player spawn helpers
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Platform")
    int32 GetPlayerSpawnTransformCount() const { return PlayerSpawnTransforms.Num(); }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Platform")
    FTransform GetPlayerSpawnTransform(int32 Index) const { return PlayerSpawnTransforms.IsValidIndex(Index) ? PlayerSpawnTransforms[Index] : FTransform::Identity; }
};
