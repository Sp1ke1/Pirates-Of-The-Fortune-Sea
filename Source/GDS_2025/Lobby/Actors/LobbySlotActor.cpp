#include "GDS_2025/Lobby/Actors/LobbySlotActor.h"

#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/StaticMeshComponent.h"

ALobbySlotActor::ALobbySlotActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	FocusAnchor = CreateDefaultSubobject<USceneComponent>(TEXT("FocusAnchor"));
	FocusAnchor->SetupAttachment(Root);
	FocusAnchor->SetRelativeLocation(FVector(0.f, 0.f, 220.f)); // above head, tweak

	CharacterMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh"));
	CharacterMesh->SetupAttachment(Root);

	ChangeText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("ChangeText"));
	ChangeText->SetupAttachment(Root);
	ChangeText->SetRelativeLocation(FVector(0.f, 80.f, 120.f));
	ChangeText->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	ChangeText->SetText(FText::FromString(TEXT("CHANGE")));
	ChangeText->SetWorldSize(24.f);

	LeftTriangle = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftTriangle"));
	LeftTriangle->SetupAttachment(Root);
	LeftTriangle->SetRelativeLocation(FVector(-35.f, 80.f, 120.f));
	LeftTriangle->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	LeftTriangle->SetGenerateOverlapEvents(false);

	RightTriangle = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightTriangle"));
	RightTriangle->SetupAttachment(Root);
	RightTriangle->SetRelativeLocation(FVector(35.f, 80.f, 120.f));
	RightTriangle->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	RightTriangle->SetGenerateOverlapEvents(false);

	// Make mouse hover traces possible by default
	CharacterMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CharacterMesh->SetCollisionResponseToAllChannels(ECR_Block);
	ChangeText->SetCollisionEnabled(ECollisionEnabled::NoCollision); // TextRender isn't great for hits; up to you

	UpdateHoverVisuals();
}

void ALobbySlotActor::ApplyData(const FLobbySlotData& Data)
{
	// Minimal stub: you will later apply:
	// - skin index -> mesh/material override / cosmetic asset
	// - control source -> show "EMPTY" / "AI" / "P1" / "MM" etc
	// - player color -> accent material, etc

	// Example: change the "CHANGE" label based on occupancy
	FString Label = TEXT("CHANGE");

	switch (Data.Control.Source)
	{
	case ELobbyControlSource::None:
		Label = TEXT("ADD");
		break;
	case ELobbyControlSource::Keyboard:
		Label = TEXT("KEYBOARD");
		break;
	case ELobbyControlSource::Gamepad:
		Label = TEXT("GAMEPAD");
		break;
	case ELobbyControlSource::AI:
		Label = TEXT("AI");
		break;
	case ELobbyControlSource::Matchmaking:
		Label = TEXT("ONLINE");
		break;
	default:
		break;
	}

	ChangeText->SetText(FText::FromString(Label));

	// You can also store Data.SkinIndex somewhere if needed.
	// For now we keep it visual-only.
}

void ALobbySlotActor::SetHovered(const bool bHovered)
{
	if (bIsHovered == bHovered)
	{
		return;
	}

	bIsHovered = bHovered;
	UpdateHoverVisuals();
}

void ALobbySlotActor::UpdateHoverVisuals()
{
	// Very simple: scale up slightly when hovered
	const float Scale = bIsHovered ? 1.1f : 1.0f;
	SetActorScale3D(FVector(Scale));
}
