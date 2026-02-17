#include "PlatformRespawnComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UPlatformRespawnComponent::UPlatformRespawnComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

bool UPlatformRespawnComponent::RespawnPlayer(APawn* Pawn, int32 Index)
{
    if (!Pawn)
    {
        return false;
    }

    if (RespawnPoints.Num() == 0)
    {
        return false;
    }

    if (Index < 0 || Index >= RespawnPoints.Num())
    {
        return false;
    }

    const FTransform& Target = RespawnPoints[Index];

    AActor* PawnActor = Cast<AActor>(Pawn);
    if (!PawnActor)
    {
        return false;
    }

    // Teleport pawn to target transform (location + rotation)
    FVector NewLocation = Target.GetLocation();
    FRotator NewRotation = Target.GetRotation().Rotator();

    // Use SetActorLocationAndRotation to attempt physics-safe teleport
    FHitResult Hit;
    PawnActor->SetActorLocationAndRotation(NewLocation, NewRotation, false, &Hit, ETeleportType::TeleportPhysics);

    // If this is a character, clear its movement velocity to avoid continued motion
    if (ACharacter* AsCharacter = Cast<ACharacter>(Pawn))
    {
        if (UCharacterMovementComponent* MoveComp = AsCharacter->GetCharacterMovement())
        {
            MoveComp->StopMovementImmediately();
        }
    }

    return true;
}

