#include "KillPlane.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GDS_2025/Game/Components/MassComponent.h"
#include "GDS_2025/Game/Components/PlatformRespawnComponent.h"

AKillPlane::AKillPlane()
{
    PrimaryActorTick.bCanEverTick = false;

    KillBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("KillBounds"));
    RootComponent = KillBounds;
    KillBounds->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    KillBounds->SetGenerateOverlapEvents(true);
}

void AKillPlane::BeginPlay()
{
    Super::BeginPlay();

    if (KillBounds)
    {
        KillBounds->OnComponentBeginOverlap.AddDynamic(this, &AKillPlane::OnKillBoundsBeginOverlap);
    }
}

void AKillPlane::OnKillBoundsBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this)
    {
        return;
    }

    // Check for MassComponent on the actor
    UMassComponent* MassComp = OtherActor->FindComponentByClass<UMassComponent>();
    if (!MassComp)
    {
        return; // ignore actors without mass
    }

    // If the actor has a PlatformRespawnComponent, attempt to respawn
    UPlatformRespawnComponent* RespawnComp = OtherActor->FindComponentByClass<UPlatformRespawnComponent>();
    APawn* Pawn = Cast<APawn>(OtherActor);

    if (RespawnComp && Pawn)
    {
        // Respawn the pawn at index 0 by default
        RespawnComp->RespawnPlayer(Pawn, 0);
    }
    else
    {
        // Otherwise destroy the actor
        OtherActor->Destroy();
    }
}
