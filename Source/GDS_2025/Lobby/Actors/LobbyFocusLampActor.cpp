#include "GDS_2025/Lobby/Actors/LobbyFocusLampActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"

ALobbyFocusLampActor::ALobbyFocusLampActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	LampMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LampMesh"));
	LampMesh->SetupAttachment(Root);
	LampMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Light = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
	Light->SetupAttachment(Root);
	Light->Intensity = 1200.f;
	Light->AttenuationRadius = 220.f;

	SetColor(FLinearColor::White);
}

void ALobbyFocusLampActor::AttachToAnchor(USceneComponent* Anchor)
{
	if (!Anchor)
	{
		return;
	}

	CurrentAnchor = Anchor;

	AttachToComponent(Anchor, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	SetActorRelativeLocation(FVector::ZeroVector);
	SetActorRelativeRotation(FRotator::ZeroRotator);
}

void ALobbyFocusLampActor::SetColor(const FLinearColor& Color)
{
	if (Light)
	{
		Light->SetLightColor(Color);
	}

	// If you want the mesh to tint too, later you can create a dynamic material instance here.
}

void ALobbyFocusLampActor::SetVisibleLamp(const bool bVisible)
{
	SetActorHiddenInGame(!bVisible);
	SetActorEnableCollision(false);
}
