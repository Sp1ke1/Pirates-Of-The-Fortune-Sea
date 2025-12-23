#include "GDS_2025/Lobby/Actors/LobbySlotActor.h"
#include "GDS_2025/Lobby/Presets/PresetLibrarySubsystem.h"
#include "Engine/GameInstance.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GDS_2025/Lobby/Framework/LobbyGameInstance.h"
#include "GDS_2025/Lobby/Devices/LobbyDeviceRegistry.h"
#include "GDS_2025/Lobby/Framework/LobbyPlayerController.h"
#include "GDS_2025/Lobby/Framework/LobbySubsystem.h"
#include "GDS_2025/Lobby/Framework/LobbyUtils.h"

#include "Engine/Engine.h"

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
	LeftTriangle->SetCollisionResponseToAllChannels(ECR_Block);

	RightTriangle = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightTriangle"));
	RightTriangle->SetupAttachment(Root);
	RightTriangle->SetRelativeLocation(FVector(35.f, 80.f, 120.f));
	RightTriangle->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	RightTriangle->SetCollisionResponseToAllChannels(ECR_Block);

	// Up/Down triangles for control cycling
	UpTriangle = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("UpTriangle"));
	UpTriangle->SetupAttachment(Root);
	UpTriangle->SetRelativeLocation(FVector(0.f, 60.f, 160.f));
	UpTriangle->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	UpTriangle->SetCollisionResponseToAllChannels(ECR_Block);

	DownTriangle = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DownTriangle"));
	DownTriangle->SetupAttachment(Root);
	DownTriangle->SetRelativeLocation(FVector(0.f, 100.f, 80.f));
	DownTriangle->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DownTriangle->SetCollisionResponseToAllChannels(ECR_Block);

	// Assign prompt
	AssignText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("AssignText"));
	AssignText->SetupAttachment(Root);
	AssignText->SetRelativeLocation(FVector(0.f, 40.f, 100.f));
	AssignText->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	AssignText->SetText(FText::FromString(TEXT("Assign Me Here (A / Enter)")));
	AssignText->SetWorldSize(20.f);

	// Skin labels
	SkinPrevLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("SkinPrevLabel"));
	SkinPrevLabel->SetupAttachment(LeftTriangle);
	SkinPrevLabel->SetRelativeLocation(FVector(0.f, 0.f, -20.f));
	SkinPrevLabel->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	SkinPrevLabel->SetText(FText::FromString(TEXT("Prev Skin (LB / <)")));
	SkinPrevLabel->SetWorldSize(14.f);

	SkinNextLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("SkinNextLabel"));
	SkinNextLabel->SetupAttachment(RightTriangle);
	SkinNextLabel->SetRelativeLocation(FVector(0.f, 0.f, -20.f));
	SkinNextLabel->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	SkinNextLabel->SetText(FText::FromString(TEXT("Next Skin (RB / >)")));
	SkinNextLabel->SetWorldSize(14.f);

	// Up/Down labels for control cycling
	UpLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("UpLabel"));
	UpLabel->SetupAttachment(UpTriangle);
	UpLabel->SetRelativeLocation(FVector(0.f, 0.f, -20.f));
	UpLabel->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	UpLabel->SetText(FText::FromString(TEXT("Control Up (UpArrow / W)")));
	UpLabel->SetWorldSize(14.f);

	DownLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("DownLabel"));
	DownLabel->SetupAttachment(DownTriangle);
	DownLabel->SetRelativeLocation(FVector(0.f, 0.f, -20.f));
	DownLabel->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	DownLabel->SetText(FText::FromString(TEXT("Control Down (DownArrow / S)")));
	DownLabel->SetWorldSize(14.f);

	// Allow traces for hover
	CharacterMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CharacterMesh->SetCollisionResponseToAllChannels(ECR_Block);

	// Bind click handlers so mouse clicks on triangles change presets.
	if (LeftTriangle)
	{
		LeftTriangle->OnClicked.AddDynamic(this, &ALobbySlotActor::OnLeftTriangleClicked);
	}
	if (RightTriangle)
	{
		RightTriangle->OnClicked.AddDynamic(this, &ALobbySlotActor::OnRightTriangleClicked);
	}

    if (UpTriangle)
    {
        UpTriangle->OnClicked.AddDynamic(this, &ALobbySlotActor::OnUpTriangleClicked);
    }
    if (DownTriangle)
    {
        DownTriangle->OnClicked.AddDynamic(this, &ALobbySlotActor::OnDownTriangleClicked);
    }

	UpdateFocusVisuals();
}

void ALobbySlotActor::OnLeftTriangleClicked(UPrimitiveComponent* /*TouchedComponent*/, FKey /*ButtonPressed*/)
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (ULobbyGameInstance* LobbyGI = Cast<ULobbyGameInstance>(GI))
		{
			if (ULobbySubsystem* Sub = LobbyGI->GetSubsystem<ULobbySubsystem>())
			{
				const FLobbyDeviceId DevId = GetDeviceIdForController(GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr, LobbyGI->GetDeviceRegistry());
				Sub->CyclePresetWithDevice(SlotIndex, -1, DevId);
			}
		}
	}
}

void ALobbySlotActor::OnRightTriangleClicked(UPrimitiveComponent* /*TouchedComponent*/, FKey /*ButtonPressed*/)
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (ULobbyGameInstance* LobbyGI = Cast<ULobbyGameInstance>(GI))
		{
			if (ULobbySubsystem* Sub = LobbyGI->GetSubsystem<ULobbySubsystem>())
			{
				const FLobbyDeviceId DevId = GetDeviceIdForController(GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr, LobbyGI->GetDeviceRegistry());
				Sub->CyclePresetWithDevice(SlotIndex, +1, DevId);
			}
		}
	}
}

void ALobbySlotActor::OnUpTriangleClicked(UPrimitiveComponent* /*TouchedComponent*/, FKey /*ButtonPressed*/)
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (ULobbyGameInstance* LobbyGI = Cast<ULobbyGameInstance>(GI))
		{
			LobbyGI->CycleSlotControl(SlotIndex, -1);
		}
	}
}

void ALobbySlotActor::OnDownTriangleClicked(UPrimitiveComponent* /*TouchedComponent*/, FKey /*ButtonPressed*/)
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (ULobbyGameInstance* LobbyGI = Cast<ULobbyGameInstance>(GI))
		{
			LobbyGI->CycleSlotControl(SlotIndex, +1);
		}
	}
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

	SetActorScale3D(FVector(Scale));
}
