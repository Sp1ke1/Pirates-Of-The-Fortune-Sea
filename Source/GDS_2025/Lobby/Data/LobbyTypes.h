#pragma once

#include "CoreMinimal.h"
#include "LobbyTypes.generated.h"

UENUM(BlueprintType)
enum class ELobbyControlSource : uint8
{
	None        UMETA(DisplayName="None"),
	Keyboard    UMETA(DisplayName="Keyboard"),
	Gamepad     UMETA(DisplayName="Gamepad"),
	AI          UMETA(DisplayName="AI"),
	Matchmaking UMETA(DisplayName="Matchmaking"),
};

UENUM(BlueprintType)
enum class ELobbyDeviceType : uint8
{
	None     UMETA(DisplayName="None"),
	Keyboard UMETA(DisplayName="Keyboard"),
	Gamepad  UMETA(DisplayName="Gamepad"),
};

USTRUCT(BlueprintType)
struct FLobbyDeviceId
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lobby")
	ELobbyDeviceType Type = ELobbyDeviceType::None;

	// For gamepads: 0,1,2,...
	// For keyboard: always 0
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lobby")
	int32 Index = 0;

	FLobbyDeviceId() = default;
	FLobbyDeviceId(const ELobbyDeviceType InType, const int32 InIndex)
		: Type(InType), Index(InIndex)
	{
	}

	friend bool operator==(const FLobbyDeviceId& A, const FLobbyDeviceId& B)
	{
		return A.Type == B.Type && A.Index == B.Index;
	}

	friend uint32 GetTypeHash(const FLobbyDeviceId& Id)
	{
		return HashCombine(::GetTypeHash(static_cast<uint8>(Id.Type)), ::GetTypeHash(Id.Index));
	}

	static FLobbyDeviceId None()
	{
		return FLobbyDeviceId(ELobbyDeviceType::None, 0);
	}

	static FLobbyDeviceId Keyboard()
	{
		return FLobbyDeviceId(ELobbyDeviceType::Keyboard, 0);
	}

	static FLobbyDeviceId Gamepad(const int32 GamepadIndex)
	{
		return FLobbyDeviceId(ELobbyDeviceType::Gamepad, GamepadIndex);
	}
};

USTRUCT(BlueprintType)
struct FLobbyControlAssignment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lobby")
	ELobbyControlSource Source = ELobbyControlSource::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lobby")
	FLobbyDeviceId DeviceId = FLobbyDeviceId::None();

	static FLobbyControlAssignment None()
	{
		FLobbyControlAssignment A;
		A.Source = ELobbyControlSource::None;
		A.DeviceId = FLobbyDeviceId::None();
		return A;
	}

	static FLobbyControlAssignment Keyboard()
	{
		FLobbyControlAssignment A;
		A.Source = ELobbyControlSource::Keyboard;
		A.DeviceId = FLobbyDeviceId::Keyboard();
		return A;
	}

	static FLobbyControlAssignment AI()
	{
		FLobbyControlAssignment A;
		A.Source = ELobbyControlSource::AI;
		A.DeviceId = FLobbyDeviceId::None();
		return A;
	}

	static FLobbyControlAssignment Matchmaking()
	{
		FLobbyControlAssignment A;
		A.Source = ELobbyControlSource::Matchmaking;
		A.DeviceId = FLobbyDeviceId::None();
		return A;
	}

	static FLobbyControlAssignment Gamepad(const int32 GamepadIndex)
	{
		FLobbyControlAssignment A;
		A.Source = ELobbyControlSource::Gamepad;
		A.DeviceId = FLobbyDeviceId::Gamepad(GamepadIndex);
		return A;
	}
};

USTRUCT(BlueprintType)
struct FLobbyControlOption
{
	GENERATED_BODY()

	// What user sees/chooses in the widget
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lobby")
	FText Label;

	// What will be applied to slot if chosen
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lobby")
	FLobbyControlAssignment Assignment;

	FLobbyControlOption() = default;
	FLobbyControlOption(const FText& InLabel, const FLobbyControlAssignment& InAssignment)
		: Label(InLabel), Assignment(InAssignment)
	{
	}
};
