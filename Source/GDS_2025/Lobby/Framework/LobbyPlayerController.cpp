#include "GDS_2025/Lobby/Framework/LobbyPlayerController.h"

#include "GDS_2025/Lobby/Framework/LobbyGameInstance.h"
#include "GDS_2025/Lobby/Framework/LobbyGameMode.h"
#include "GDS_2025/Lobby/Actors/LobbyFocusLampActor.h"
#include "GDS_2025/Lobby/Actors/LobbySlotActor.h"

#include "Engine/World.h"

// Enhanced Input
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"

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
	AddLobbyMappingContext();

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

void ALobbyPlayerController::AddLobbyMappingContext()
{
	if (!LobbyMappingContext)
	{
		UE_LOG(LogTemp, Warning, TEXT("[LobbyPC] LobbyMappingContext is not set (IMC_Lobby)."));
		return;
	}

	// For local player only
	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsys = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			Subsys->AddMappingContext(LobbyMappingContext, LobbyMappingPriority);
		}
	}
}

void ALobbyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Must be EnhancedInputComponent
	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EIC)
	{
		UE_LOG(LogTemp, Error, TEXT("[LobbyPC] InputComponent is not UEnhancedInputComponent. Ensure Enhanced Input is enabled."));
		return;
	}

	// Bind actions (Triggered is usually fine for buttons; Started is also OK)
	if (IA_FocusLeft)  EIC->BindAction(IA_FocusLeft, ETriggerEvent::Started, this, &ALobbyPlayerController::OnFocusLeft);
	if (IA_FocusRight) EIC->BindAction(IA_FocusRight, ETriggerEvent::Started, this, &ALobbyPlayerController::OnFocusRight);

	if (IA_Confirm)    EIC->BindAction(IA_Confirm, ETriggerEvent::Started, this, &ALobbyPlayerController::OnConfirm);
	if (IA_Cancel)     EIC->BindAction(IA_Cancel, ETriggerEvent::Started, this, &ALobbyPlayerController::OnCancel);

	if (IA_SkinPrev)   EIC->BindAction(IA_SkinPrev, ETriggerEvent::Started, this, &ALobbyPlayerController::OnSkinPrev);
	if (IA_SkinNext)   EIC->BindAction(IA_SkinNext, ETriggerEvent::Started, this, &ALobbyPlayerController::OnSkinNext);

	if (IA_ControlUp)   EIC->BindAction(IA_ControlUp,   ETriggerEvent::Started, this, &ALobbyPlayerController::OnControlUp);
	if (IA_ControlDown) EIC->BindAction(IA_ControlDown, ETriggerEvent::Started, this, &ALobbyPlayerController::OnControlDown);
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
}

void ALobbyPlayerController::OnControlUp(const FInputActionValue&)
{
	CacheLobbyRefs();
	if (LobbyGI) LobbyGI->CycleSlotControl(FocusedSlotIndex, -1);
}

void ALobbyPlayerController::OnControlDown(const FInputActionValue&)
{
	CacheLobbyRefs();
	if (LobbyGI) LobbyGI->CycleSlotControl(FocusedSlotIndex, +1);
}

void ALobbyPlayerController::EnsureFocusLamp()
{
	if (FocusLamp || !GetWorld())
	{
		return;
	}

	if (!FocusLampClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[LobbyPC] FocusLampClass is not set. Assign BP_LobbyFocusLamp."));
		return;
	}

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FocusLamp = GetWorld()->SpawnActor<ALobbyFocusLampActor>(FocusLampClass, FVector::ZeroVector, FRotator::ZeroRotator, Params);
	if (FocusLamp)
	{
		FocusLamp->SetColor(FocusLampColor);
	}
}

void ALobbyPlayerController::MoveLampToFocusedSlot()
{
	if (!LobbyGM)
	{
		return;
	}

	ALobbySlotActor* NewSlot = LobbyGM->GetSlotActor(FocusedSlotIndex);

	// Update active focus ref-count
	if (FocusedSlotActor.Get() != NewSlot)
	{
		if (FocusedSlotActor)
		{
			FocusedSlotActor->RemoveActiveFocus();
		}

		FocusedSlotActor = NewSlot;

		if (FocusedSlotActor)
		{
			FocusedSlotActor->AddActiveFocus();
		}
	}

	// Move lamp
	if (FocusLamp && FocusedSlotActor && FocusedSlotActor->GetFocusAnchor())
	{
		FocusLamp->AttachToAnchor(FocusedSlotActor->GetFocusAnchor());
	}
}

void ALobbyPlayerController::SetFocusedSlotIndex(const int32 NewIndex)
{
	FocusedSlotIndex = WrapSlotIndex(NewIndex);
	ClampAndApplyFocus();
	MoveLampToFocusedSlot();
}

// ---- Enhanced Input handlers ----

void ALobbyPlayerController::OnFocusLeft(const FInputActionValue& Value)
{
	// Boolean actions: Value.Get<bool>() is fine, but Started already implies pressed.
	SetFocusedSlotIndex(FocusedSlotIndex - 1);
}

void ALobbyPlayerController::OnFocusRight(const FInputActionValue& Value)
{
	SetFocusedSlotIndex(FocusedSlotIndex + 1);
}

void ALobbyPlayerController::OnConfirm(const FInputActionValue& Value)
{
	CacheLobbyRefs();
	if (!LobbyGM) return;

	LobbyGM->OpenAssignControlUI(this, FocusedSlotIndex);
}

void ALobbyPlayerController::OnCancel(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Log, TEXT("[LobbyPC] Cancel pressed"));
}

void ALobbyPlayerController::OnSkinPrev(const FInputActionValue& Value)
{
	CacheLobbyRefs();
	if (!LobbyGI) return;

	LobbyGI->CyclePreset(FocusedSlotIndex, -1);
}

void ALobbyPlayerController::OnSkinNext(const FInputActionValue& Value)
{
	CacheLobbyRefs();
	if (!LobbyGI) return;

	LobbyGI->CyclePreset(FocusedSlotIndex, +1);
}

// ---- Mouse hover (unchanged) ----

void ALobbyPlayerController::UpdateMouseHoverFocus()
{
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
			SetFocusedSlotIndex(HoveredSlot->SlotIndex);
		}
	}
}

void ALobbyPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	UpdateMouseHoverFocus();
}
