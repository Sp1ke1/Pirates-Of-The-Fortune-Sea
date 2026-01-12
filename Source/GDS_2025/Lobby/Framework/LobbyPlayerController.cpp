#include "GDS_2025/Lobby/Framework/LobbyPlayerController.h"

#include "GDS_2025/Lobby/Framework/LobbyGameInstance.h"
#include "GDS_2025/Lobby/Framework/LobbySubsystem.h"
#include "GDS_2025/Lobby/Framework/LobbyUtils.h"
#include "GDS_2025/Lobby/Framework/LobbyGameMode.h"
#include "GDS_2025/Lobby/Devices/LobbyDeviceRegistry.h"
#include "GDS_2025/Lobby/Actors/LobbyFocusLampActor.h"
#include "GDS_2025/Lobby/Actors/LobbySlotActor.h"
#include "GDS_2025/Lobby/Actors/LobbyStartCannonActor.h"

#include "Engine/World.h"

// Enhanced Input
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "EngineUtils.h"

ALobbyPlayerController::ALobbyPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void ALobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// IMPORTANT: Set input mode to allow both game and UI input
	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	
	SetShowMouseCursor(true);

	CacheLobbyRefs();
	AddLobbyMappingContext();

	EnsureFocusLamp();
	ClampAndApplyFocus();
	MoveLampToFocusedSlot();

	// Try to find the cannon actor in the level (first found)
	if (GetWorld())
	{
		for (TActorIterator<ALobbyStartCannonActor> It(GetWorld()); It; ++It)
		{
			StartCannon = *It;
			break;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[LobbyPC] BeginPlay completed. InputMode set to GameAndUI, MouseCursor visible: %d"), bShowMouseCursor);
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
		UE_LOG(LogTemp, Error, TEXT("[LobbyPC] LobbyMappingContext is not set (IMC_Lobby). Assign it in Blueprint or C++ default properties."));
		return;
	}

	// For local player only
	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsys = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			Subsys->AddMappingContext(LobbyMappingContext, LobbyMappingPriority);
			UE_LOG(LogTemp, Log, TEXT("[LobbyPC] Added LobbyMappingContext with priority %d"), LobbyMappingPriority);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[LobbyPC] Failed to get UEnhancedInputLocalPlayerSubsystem. Enhanced Input may not be properly initialized."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[LobbyPC] No LocalPlayer found. Cannot add mapping context."));
	}
}

void ALobbyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Must be EnhancedInputComponent
	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EIC)
	{
		UE_LOG(LogTemp, Error, TEXT("[LobbyPC] InputComponent is not UEnhancedInputComponent. Check Project Settings -> Input -> Default Player Input Class is set to EnhancedPlayerInput."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[LobbyPC] SetupInputComponent: Binding Enhanced Input Actions..."));

	// Bind actions (Triggered is usually fine for buttons; Started is also OK)
	int32 BoundCount = 0;

	if (IA_FocusLeft)
	{
		EIC->BindAction(IA_FocusLeft, ETriggerEvent::Started, this, &ALobbyPlayerController::OnFocusLeft);
		BoundCount++;
		UE_LOG(LogTemp, Log, TEXT("[LobbyPC] Bound IA_FocusLeft"));
	}
	else { UE_LOG(LogTemp, Warning, TEXT("[LobbyPC] IA_FocusLeft is NULL - not bound!")); }

	if (IA_FocusRight)
	{
		EIC->BindAction(IA_FocusRight, ETriggerEvent::Started, this, &ALobbyPlayerController::OnFocusRight);
		BoundCount++;
		UE_LOG(LogTemp, Log, TEXT("[LobbyPC] Bound IA_FocusRight"));
	}
	else { UE_LOG(LogTemp, Warning, TEXT("[LobbyPC] IA_FocusRight is NULL - not bound!")); }

	if (IA_Confirm)
	{
		EIC->BindAction(IA_Confirm, ETriggerEvent::Started, this, &ALobbyPlayerController::OnConfirm);
		BoundCount++;
		UE_LOG(LogTemp, Log, TEXT("[LobbyPC] Bound IA_Confirm"));
	}
	else { UE_LOG(LogTemp, Warning, TEXT("[LobbyPC] IA_Confirm is NULL - not bound!")); }

	if (IA_Cancel)
	{
		EIC->BindAction(IA_Cancel, ETriggerEvent::Started, this, &ALobbyPlayerController::OnCancel);
		BoundCount++;
		UE_LOG(LogTemp, Log, TEXT("[LobbyPC] Bound IA_Cancel"));
	}
	else { UE_LOG(LogTemp, Warning, TEXT("[LobbyPC] IA_Cancel is NULL - not bound!")); }

	if (IA_SkinPrev)
	{
		EIC->BindAction(IA_SkinPrev, ETriggerEvent::Started, this, &ALobbyPlayerController::OnSkinPrev);
		BoundCount++;
		UE_LOG(LogTemp, Log, TEXT("[LobbyPC] Bound IA_SkinPrev"));
	}
	else { UE_LOG(LogTemp, Warning, TEXT("[LobbyPC] IA_SkinPrev is NULL - not bound!")); }

	if (IA_SkinNext)
	{
		EIC->BindAction(IA_SkinNext, ETriggerEvent::Started, this, &ALobbyPlayerController::OnSkinNext);
		BoundCount++;
		UE_LOG(LogTemp, Log, TEXT("[LobbyPC] Bound IA_SkinNext"));
	}
	else { UE_LOG(LogTemp, Warning, TEXT("[LobbyPC] IA_SkinNext is NULL - not bound!")); }

	if (IA_LobbyStartGame)
	{
		EIC->BindAction(IA_LobbyStartGame, ETriggerEvent::Started, this, &ALobbyPlayerController::StartHoldStartGame);
		EIC->BindAction(IA_LobbyStartGame, ETriggerEvent::Completed, this, &ALobbyPlayerController::StopHoldStartGame);
		EIC->BindAction(IA_LobbyStartGame, ETriggerEvent::Canceled, this, &ALobbyPlayerController::StopHoldStartGame);
		BoundCount += 3;
		UE_LOG(LogTemp, Log, TEXT("[LobbyPC] Bound IA_LobbyStartGame (Started/Completed/Canceled)"));
	}
	else { UE_LOG(LogTemp, Warning, TEXT("[LobbyPC] IA_LobbyStartGame is NULL - not bound!")); }

	if (IA_ControlUp)
	{
		EIC->BindAction(IA_ControlUp, ETriggerEvent::Started, this, &ALobbyPlayerController::OnControlUp);
		BoundCount++;
		UE_LOG(LogTemp, Log, TEXT("[LobbyPC] Bound IA_ControlUp"));
	}
	else { UE_LOG(LogTemp, Warning, TEXT("[LobbyPC] IA_ControlUp is NULL - not bound!")); }

	if (IA_ControlDown)
	{
		EIC->BindAction(IA_ControlDown, ETriggerEvent::Started, this, &ALobbyPlayerController::OnControlDown);
		BoundCount++;
		UE_LOG(LogTemp, Log, TEXT("[LobbyPC] Bound IA_ControlDown"));
	}
	else { UE_LOG(LogTemp, Warning, TEXT("[LobbyPC] IA_ControlDown is NULL - not bound!")); }

	UE_LOG(LogTemp, Log, TEXT("[LobbyPC] SetupInputComponent completed. Total actions bound: %d"), BoundCount);
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
	UE_LOG(LogTemp, VeryVerbose, TEXT("[LobbyPC] OnFocusLeft triggered"));
	// Boolean actions: Value.Get<bool>() is fine, but Started already implies pressed.
	SetFocusedSlotIndex(FocusedSlotIndex - 1);
}

void ALobbyPlayerController::OnFocusRight(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, VeryVerbose, TEXT("[LobbyPC] OnFocusRight triggered"));
	SetFocusedSlotIndex(FocusedSlotIndex + 1);
}

void ALobbyPlayerController::OnConfirm(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Log, TEXT("[LobbyPC] OnConfirm triggered for slot %d"), FocusedSlotIndex);
	CacheLobbyRefs();
	if (!LobbyGI)
	{
		UE_LOG(LogTemp, Warning, TEXT("[LobbyPC] OnConfirm: LobbyGI is null!"));
		return;
	}

	// Assign the physical device that triggered this PlayerController to the focused slot.
	const bool bAssigned = LobbyGI->AssignPhysicalDeviceFromController(this, FocusedSlotIndex);
	if (!bAssigned)
	{
		UE_LOG(LogTemp, Warning, TEXT("[LobbyPC] Failed to assign physical device to slot %d"), FocusedSlotIndex);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[LobbyPC] Successfully assigned device to slot %d"), FocusedSlotIndex);
	}
}

void ALobbyPlayerController::OnCancel(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Log, TEXT("[LobbyPC] Cancel pressed"));
}

void ALobbyPlayerController::StartHoldStartGame(const FInputActionValue& Value)
{
	if (!StartCannon)
	{
		return;
	}

	// Determine device id and begin hold
	{
		const FLobbyDeviceId DevId = GetDeviceIdForController(this, LobbyGI ? LobbyGI->GetDeviceRegistry() : nullptr);
		StartCannon->BeginHold(DevId);
	}
}

void ALobbyPlayerController::StopHoldStartGame(const FInputActionValue& Value)
{
	if (StartCannon)
	{
		StartCannon->EndHold();
	}
}

void ALobbyPlayerController::OnSkinPrev(const FInputActionValue& Value)
{
	CacheLobbyRefs();
	if (!LobbyGI) return;

	{
		const FLobbyDeviceId DevId = GetDeviceIdForController(this, LobbyGI ? LobbyGI->GetDeviceRegistry() : nullptr);
		if (LobbyGI)
		{
			if (ULobbySubsystem* Sub = LobbyGI->GetSubsystem<ULobbySubsystem>())
			{
				Sub->CyclePresetWithDevice(FocusedSlotIndex, -1, DevId);
			}
		}
	}
}

void ALobbyPlayerController::OnSkinNext(const FInputActionValue& Value)
{
	CacheLobbyRefs();
	if (!LobbyGI) return;

	{
		const FLobbyDeviceId DevId = GetDeviceIdForController(this, LobbyGI ? LobbyGI->GetDeviceRegistry() : nullptr);
		if (LobbyGI)
		{
			if (ULobbySubsystem* Sub = LobbyGI->GetSubsystem<ULobbySubsystem>())
			{
				Sub->CyclePresetWithDevice(FocusedSlotIndex, +1, DevId);
			}
		}
	}
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
