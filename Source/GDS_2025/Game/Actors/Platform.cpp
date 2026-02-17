#include "Platform.h"
#include "Components/SceneComponent.h"
#include "GDS_2025/Game/Components/PlatformMassBoundsComponent.h"
#include "GDS_2025/Game/Components/PlatformRotationComponent.h"

APlatform::APlatform()
{
    PrimaryActorTick.bCanEverTick = false;

    RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootScene;

    RotationComponent = CreateDefaultSubobject<UPlatformRotationComponent>(TEXT("RotationComponent"));
    // RotationComponent is an ActorComponent, no attachment needed
}

void APlatform::BeginPlay()
{
    Super::BeginPlay();

    // Gather all MassBounds components that might have been added in Blueprint or C++
    TArray<UPlatformMassBoundsComponent*> FoundBounds;
    GetComponents<UPlatformMassBoundsComponent>(FoundBounds);

    // Merge unique entries into MassBoundsComponents
    for (UPlatformMassBoundsComponent* B : FoundBounds)
    {
        if (B && !MassBoundsComponents.Contains(B))
        {
            MassBoundsComponents.Add(B);
        }
    }

    // Wire up rotation component for each bounds
    for (UPlatformMassBoundsComponent* B : MassBoundsComponents)
    {
        if (B && RotationComponent)
        {
            B->SetRotationComponent(RotationComponent);
        }
    }
}
