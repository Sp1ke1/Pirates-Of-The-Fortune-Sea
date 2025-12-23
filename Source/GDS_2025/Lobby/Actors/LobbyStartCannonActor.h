#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GDS_2025/Lobby/Data/LobbyTypes.h"
#include "LobbyStartCannonActor.generated.h"

class UStaticMeshComponent;
class USceneComponent;
class UPointLightComponent;
class UTextRenderComponent;
class ALobbyGameMode;

UCLASS()
class ALobbyStartCannonActor : public AActor
{
    GENERATED_BODY()

public:
    ALobbyStartCannonActor();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    // Public API for controller
    void BeginHold(const FLobbyDeviceId& RequestingDevice);
    void EndHold();
    bool IsBusyFiring() const { return bHasFiredThisHold; }

    UFUNCTION(BlueprintImplementableEvent, Category = "Lobby")
    void FireEffects();

protected:
    // Components
    UPROPERTY(VisibleAnywhere, Category="Cannon")
    USceneComponent* Root;

    UPROPERTY(VisibleAnywhere, Category="Cannon")
    UStaticMeshComponent* CannonMesh;

    // Target contact point (place this near the candle flame in the level)
    UPROPERTY(EditAnywhere, Category="Cannon")
    USceneComponent* CandleContactPoint;

    // Optional candle visuals
    UPROPERTY(EditAnywhere, Category="Cannon")
    UStaticMeshComponent* CandleMesh;

    UPROPERTY(EditAnywhere, Category="Cannon")
    UPointLightComponent* CandleLight;

    // UI text attached to the cannon that shows start prompt and device indicator
    UPROPERTY(VisibleAnywhere, Category="Cannon")
    UTextRenderComponent* StartText;

    // Config
    UPROPERTY(EditAnywhere, Category="Cannon|Config")
    float HoldDuration = 1.5f; // seconds to reach candle when holding

    UPROPERTY(EditAnywhere, Category="Cannon|Config")
    float ReturnSpeed = 2.5f; // alpha/sec return speed

    UPROPERTY(EditAnywhere, Category="Cannon|Config")
    float ContactDistance = 8.0f; // world units

    // State
    bool bIsHoldingStart = false;
    bool bHasFiredThisHold = false;
    float HoldAlpha = 0.0f; // 0..1 progress from initial -> contact

    // Last requesting device (used to update text)
    FLobbyDeviceId LastRequestingDevice;

    // Cached transforms
    FVector InitialLocation;
    FVector TargetLocation;

    // Handle for delayed start call
    FTimerHandle FireDelayHandle;

    // Internal helpers
    void NotifyStartRequested();
    void UpdateCannonTransform();
};
