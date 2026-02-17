#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KillPlane.generated.h"

class UBoxComponent;
class UMassComponent;
class UPlatformRespawnComponent;

UCLASS()
class GDS_2025_API AKillPlane : public AActor
{
    GENERATED_BODY()

public:
    AKillPlane();

    // Bounding box that represents the kill area
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "KillPlane")
    UBoxComponent* KillBounds;

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnKillBoundsBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};

