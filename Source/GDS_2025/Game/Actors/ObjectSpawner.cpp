#include "ObjectSpawner.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"
#include "Components/PrimitiveComponent.h"

AObjectSpawner::AObjectSpawner()
{
    PrimaryActorTick.bCanEverTick = false;

    SpawnBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnBounds"));
    RootComponent = SpawnBounds;
    SpawnBounds->SetCollisionProfileName(TEXT("OverlapAll"));
}

void AObjectSpawner::BeginPlay()
{
    Super::BeginPlay();

    // Start periodic spawning if requested
    if (bSpawnOnBeginPlay && SpawnRate > KINDA_SMALL_NUMBER)
    {
        StartSpawningTimer();
    }
}

void AObjectSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopSpawningTimer();
    Super::EndPlay(EndPlayReason);
}

FVector AObjectSpawner::GetRandomPointInBounds() const
{
    if (!SpawnBounds)
    {
        return GetActorLocation();
    }

    FVector BoxExtent = SpawnBounds->Bounds.BoxExtent;

    FVector LocalRandom = UKismetMathLibrary::RandomPointInBoundingBox(FVector::ZeroVector, BoxExtent);
    // Transform LocalRandom (which is in local space) to world space
    return SpawnBounds->GetComponentTransform().TransformPosition(LocalRandom);
}

void AObjectSpawner::SpawnRandom()
{
    if (SpawnableClasses.Num() == 0 || !GetWorld())
    {
        return;
    }

    int32 Index = FMath::RandRange(0, SpawnableClasses.Num() - 1);
    TSubclassOf<AActor> ToSpawn = SpawnableClasses[Index];
    if (!ToSpawn)
    {
        return;
    }

    FVector SpawnLocation = GetRandomPointInBounds();
    FRotator SpawnRotation = FRotator::ZeroRotator;

    FActorSpawnParameters Params;
    Params.Owner = this;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    GetWorld()->SpawnActor<AActor>(ToSpawn, SpawnLocation, SpawnRotation, Params);
}

void AObjectSpawner::SpawnTimerTick()
{
    // Spawn a single actor each tick
    SpawnRandom();
}

void AObjectSpawner::StartSpawningTimer()
{
    if (!GetWorld() || SpawnRate <= KINDA_SMALL_NUMBER)
    {
        return;
    }

    // If already active, do nothing
    if (GetWorld()->GetTimerManager().IsTimerActive(SpawnTimerHandle))
    {
        return;
    }

    GetWorld()->GetTimerManager().SetTimer(SpawnTimerHandle, this, &AObjectSpawner::SpawnTimerTick, SpawnRate, true);
}

void AObjectSpawner::StopSpawningTimer()
{
    if (!GetWorld())
    {
        return;
    }

    if (GetWorld()->GetTimerManager().IsTimerActive(SpawnTimerHandle))
    {
        GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
    }
}
