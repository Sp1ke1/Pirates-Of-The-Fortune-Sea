#include "GDS_2025/Lobby/Actors/LobbyStartCannonActor.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/Engine.h"
#include "GDS_2025/Lobby/Framework/LobbyGameMode.h"

ALobbyStartCannonActor::ALobbyStartCannonActor()
{
    PrimaryActorTick.bCanEverTick = true;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    CannonMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CannonMesh"));
    CannonMesh->SetupAttachment(Root);

    // Start prompt text
    StartText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("StartText"));
    StartText->SetupAttachment(CannonMesh);
    StartText->SetText(FText::FromString(TEXT("StartGame (Space)")));
    StartText->SetHorizontalAlignment(EHTA_Center);
    StartText->SetWorldSize(24.f);

    CandleContactPoint = CreateDefaultSubobject<USceneComponent>(TEXT("CandleContactPoint"));
    CandleContactPoint->SetupAttachment(Root);

    CandleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CandleMesh"));
    CandleMesh->SetupAttachment(Root);

    CandleLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("CandleLight"));
    CandleLight->SetupAttachment(Root);
    CandleLight->SetIntensity(800.f);
}

void ALobbyStartCannonActor::BeginPlay()
{
    Super::BeginPlay();

    InitialLocation = CannonMesh ? CannonMesh->GetComponentLocation() : GetActorLocation();

    if (CandleContactPoint)
    {
        TargetLocation = CandleContactPoint->GetComponentLocation();
    }
    else
    {
        // Fallback: use actor forward direction
        TargetLocation = InitialLocation + GetActorForwardVector() * 200.f;
    }
}

void ALobbyStartCannonActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (bHasFiredThisHold)
    {
        return; // do not move after fired
    }

    if (bIsHoldingStart)
    {
        if (HoldDuration <= 0.f)
        {
            HoldAlpha = 1.f;
        }
        else
        {
            HoldAlpha = FMath::Min(1.f, HoldAlpha + (DeltaSeconds / HoldDuration));
        }

        UpdateCannonTransform();

        // Check contact
        const float DistSq = FVector::DistSquared(CannonMesh->GetComponentLocation(), TargetLocation);
        if (DistSq <= (ContactDistance * ContactDistance) && !bHasFiredThisHold)
        {
            bHasFiredThisHold = true;
            FireEffects();
            // call start after 2 seconds
            GetWorld()->GetTimerManager().SetTimer(FireDelayHandle, this, &ALobbyStartCannonActor::NotifyStartRequested, 2.f, false);
        }
    }
    else
    {
        // returning smoothly
        if (HoldAlpha > 0.f)
        {
            HoldAlpha = FMath::Max(0.f, HoldAlpha - (ReturnSpeed * DeltaSeconds / FMath::Max(HoldDuration, 0.0001f)));
            UpdateCannonTransform();
        }
    }
}

void ALobbyStartCannonActor::BeginHold(const FLobbyDeviceId& RequestingDevice)
{
    if (bHasFiredThisHold)
    {
        return; // already fired this hold
    }
    bIsHoldingStart = true;
    LastRequestingDevice = RequestingDevice;

    // Update prompt to show controller indicator (X) or keyboard (space)
    //if (StartText)
    //{
    //    const bool bIsGamepad = (RequestingDevice.Type == ELobbyDeviceType::Gamepad);
    //    const FString Suffix = bIsGamepad ? TEXT("X)") : TEXT("Space)");
    //    StartText->SetText(FText::FromString(FString::Printf(TEXT("StartGame (%s"), *Suffix)));
    //}
}

void ALobbyStartCannonActor::EndHold()
{
    bIsHoldingStart = false;
    // If fired, we ignore release; otherwise cannon will return in Tick
}

void ALobbyStartCannonActor::UpdateCannonTransform()
{
    if (!CannonMesh)
    {
        return;
    }

    const FVector NewLoc = FMath::Lerp(InitialLocation, TargetLocation, HoldAlpha);
    CannonMesh->SetWorldLocation(NewLoc);
}

//void ALobbyStartCannonActor::FireEffects()
//{
//    // Placeholder: play sound, spawn particles, etc.
//    UE_LOG(LogTemp, Log, TEXT("[StartCannon] Fired at candle!"));
//}

void ALobbyStartCannonActor::NotifyStartRequested()
{
    // Notify GameMode to try start the game
    UWorld* W = GetWorld();
    if (!W)
    {
        return;
    }

    if (ALobbyGameMode* GM = Cast<ALobbyGameMode>(W->GetAuthGameMode()))
    {
        GM->OnStartGameFired();
    }
}
