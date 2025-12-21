#include "GDS_2025/Lobby/Actors/LobbySlotActor.h"
#include "GDS_2025/Lobby/Presets/PresetLibrarySubsystem.h"
#include "Engine/GameInstance.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GDS_2025/Lobby/Framework/LobbyGameInstance.h"
#include "GDS_2025/Lobby/Devices/LobbyDeviceRegistry.h"

ALobbySlotActor::ALobbySlotActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	FocusAnchor = CreateDefaultSubobject<USceneComponent>(TEXT("FocusAnchor"));
	FocusAnchor->SetupAttachment(Root);
	FocusAnchor->SetRelativeLocation(FVector(0.f, 0.f, 220.f));

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

	RightTriangle = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightTriangle"));
	RightTriangle->SetupAttachment(Root);
	RightTriangle->SetRelativeLocation(FVector(35.f, 80.f, 120.f));
	RightTriangle->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	// Allow traces for hover
	CharacterMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CharacterMesh->SetCollisionResponseToAllChannels(ECR_Block);

	UpdateFocusVisuals();
}

void ALobbySlotActor::ApplyData(const FLobbySlotData& Data)
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (ULobbyGameInstance* LobbyGI = Cast<ULobbyGameInstance>(GI))
		{
			if (ULobbyDeviceRegistry* Reg = LobbyGI->GetDeviceRegistry())
			{
				ChangeText->SetText(Reg->ToDisplayText(Data.Control));
			}
		}
	}

	// Apply preset mesh
	if (CharacterMesh)
	{
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UPresetLibrarySubsystem* Lib = GI->GetSubsystem<UPresetLibrarySubsystem>())
			{
				Lib->ApplyPresetToMeshById(Data.SelectedPresetId, CharacterMesh);
			}
		}
	}
}

void ALobbySlotActor::SetHovered(const bool bHovered)
{
	if (bIsHovered == bHovered)
	{
		return;
	}

	bIsHovered = bHovered;
	UpdateFocusVisuals();
}

void ALobbySlotActor::AddActiveFocus()
{
	ActiveFocusCount = FMath::Max(0, ActiveFocusCount + 1);
	UpdateFocusVisuals();
}

void ALobbySlotActor::RemoveActiveFocus()
{
	ActiveFocusCount = FMath::Max(0, ActiveFocusCount - 1);
	UpdateFocusVisuals();
}

void ALobbySlotActor::UpdateFocusVisuals()
{
	const bool bActive = (ActiveFocusCount > 0);

	float Scale = 1.0f;
	if (bActive)
	{
		Scale *= ActiveScale;
	}
	if (bIsHovered)
	{
		Scale *= HoverScale;
	}

	SetActorScale3D(FVector(Scale));
}
