#include "GDS_2025/Lobby/Framework/LobbyPlayerController.h"

#include "GDS_2025/Lobby/Framework/LobbyGameInstance.h"
#include "GDS_2025/Lobby/Framework/LobbyGameMode.h"
#include "GDS_2025/Lobby/Actors/LobbyFocusLampActor.h"
#include "GDS_2025/Lobby/Actors/LobbySlotActor.h"

#include "Engine/World.h"

ALobbyPlayerController::ALobbyPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void ALobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	CacheLobbyRefs();
	EnsureFocusLamp();
	ClampAndApplyFocus();
	MoveLampToFocusedSlot();
}

void ALobbyPlayerController::CacheLobbyRefs()
{
	LobbyGI = Cast<ULobbyGameInstance>(GetGameInstance());
	LobbyGM = GetWorld() ? Cast<ALobbyGameMode>(GetWorld()->GetAuthGameMode()) : nullptr;

	if (!LobbyGI)
	{
		UE_LOG(LogTemp, Error, TEXT("[LobbyPC] GameInstance is not ULobbyGameInstance."));
	}
	if (!LobbyGM)
	{
		UE_LOG(LogTemp, Warning, TEXT("[LobbyPC] GameMode is not ALobbyGameMode (or not authority)."));
	}
}

void ALobbyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	check(InputComponent);

	// Action Mappings required in Project Settings -> Input (legacy):
	// Lobby_Confirm, Lobby_Cancel, Lobby_FocusLeft, Lobby_FocusRight, Lobby_SkinPrev, Lobby_SkinNext

	InputComponent->BindAction(TEXT("Lobby_FocusLeft"), IE_Pressed, this, &ALobbyPlayerController::Input_FocusLeft);
	InputComponent->BindAction(TEXT("Lobby_FocusRight"), IE_Pressed, this, &ALobbyPlayerController::Input_FocusRight);

	InputComponent->BindAction(TEXT("Lobby_Confirm"), IE_Pressed, this, &ALobbyPlayerController::Input_Confirm);
	InputComponent->BindAction(TEXT("Lobby_Cancel"), IE_Pressed, this, &ALobbyPlayerController::Input_Cancel);

	InputComponent->BindAction(TEXT("Lobby_SkinPrev"), IE_Pressed, this, &ALobbyPlayerController::Input_SkinPrev);
	InputComponent->BindAction(TEXT("Lobby_SkinNext"), IE_Pressed, this, &ALobbyPlayerController::Input_SkinNext);
}

int32 ALobbyPlayerController::WrapSlotIndex(const int32 Index) const
{
	const int32 N = ULobbyGameInstance::NumLobbySlots;
	if (N <= 0) return 0;

	int32 Result = Index % N;
	if (Result < 0) Result += N;
	return Result;
}

void ALobbyPlayerController::ClampAndApplyFocus()
{
	FocusedSlotIndex = WrapSlotIndex(FocusedSlotIndex);
	UE_LOG(LogTemp, Log, TEXT("[LobbyPC] FocusedSlotIndex=%d"), FocusedSlotIndex);
}

void ALobbyPlayerController::EnsureFocusLamp()
{
	if (FocusLamp || !GetWorld())
	{
		return;
	}

	if (!FocusLampClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[LobbyPC] FocusLampClass is not set. Assign it in BP or defaults."));
		return;
	}

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FocusLamp = GetWorld()->SpawnActor<ALobbyFocusLampActor>(
		FocusLampClass,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		Params
	);

	if (FocusLamp)
	{
		FocusLamp->SetColor(FocusLampColor);
	}
}

void ALobbyPlayerController::MoveLampToFocusedSlot()
{
	if (!LobbyGM || !FocusLamp)
	{
		return;
	}

	ALobbySlotActor* SlotActor = LobbyGM->GetSlotActor(FocusedSlotIndex);
	if (SlotActor && SlotActor->GetFocusAnchor())
	{
		FocusLamp->AttachToAnchor(SlotActor->GetFocusAnchor());
	}
}

void ALobbyPlayerController::SetFocusedSlotIndex(const int32 NewIndex)
{
	FocusedSlotIndex = WrapSlotIndex(NewIndex);
	ClampAndApplyFocus();
	MoveLampToFocusedSlot();
}

void ALobbyPlayerController::Input_FocusLeft()
{
	SetFocusedSlotIndex(FocusedSlotIndex - 1);
}

void ALobbyPlayerController::Input_FocusRight()
{
	SetFocusedSlotIndex(FocusedSlotIndex + 1);
}

void ALobbyPlayerController::Input_Confirm()
{
	CacheLobbyRefs();
	if (!LobbyGM) return;

	LobbyGM->OpenAssignControlUI(this, FocusedSlotIndex);
}

void ALobbyPlayerController::Input_Cancel()
{
	UE_LOG(LogTemp, Log, TEXT("[LobbyPC] Cancel pressed"));
}

void ALobbyPlayerController::Input_SkinPrev()
{
	CacheLobbyRefs();
	if (!LobbyGI) return;

	LobbyGI->CycleSkin(FocusedSlotIndex, -1);
}

void ALobbyPlayerController::Input_SkinNext()
{
	CacheLobbyRefs();
	if (!LobbyGI) return;

	LobbyGI->CycleSkin(FocusedSlotIndex, +1);
}

void ALobbyPlayerController::UpdateMouseHoverFocus()
{
	// If you don't want mouse to "steal" focus from gamepad later,
	// we can gate this by InputOwner == Mouse. For now it's always enabled when cursor is shown.
	if (!bShowMouseCursor)
	{
		return;
	}

	FHitResult Hit;
	const bool bHit = GetHitResultUnderCursor(ECC_Visibility, true, Hit);

	ALobbySlotActor* HitSlot = bHit ? Cast<ALobbySlotActor>(Hit.GetActor()) : nullptr;

	if (HoveredSlot.Get() != HitSlot)
	{
		if (HoveredSlot)
		{
			HoveredSlot->SetHovered(false);
		}

		HoveredSlot = HitSlot;

		if (HoveredSlot)
		{
			HoveredSlot->SetHovered(true);

			// Switch focus to hovered slot
			SetFocusedSlotIndex(HoveredSlot->SlotIndex);
		}
	}
}

void ALobbyPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	UpdateMouseHoverFocus();
}
