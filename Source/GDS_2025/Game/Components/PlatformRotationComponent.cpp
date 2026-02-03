// Fill out your copyright notice in the Description page of Project Settings.


#include "PlatformRotationComponent.h"


UPlatformRotationComponent::UPlatformRotationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UPlatformRotationComponent::AddDeltaRotation(float InDeltaRotation)
{
	FRotator CurrentRotation = RotationRoot->GetRelativeRotation();
	const float CurrentAngle = CurrentRotation.GetComponentForAxis(RotationAxis);
	const float Angle = FMath::Clamp(-MaxRotation, MaxRotation, InDeltaRotation + CurrentAngle);
	CurrentRotation.SetComponentForAxis(RotationAxis, Angle);
	if ( FMath::Abs ( Angle ) >= MaxRotation )
	{
		OnMaxAngleReachedDelegate . Broadcast ( Angle > 0 ); 
	}
}

float UPlatformRotationComponent::GetRotationSpeed() const
{
	return RotationSpeed;
}

void UPlatformRotationComponent::InitializeComponent()
{
	Super::InitializeComponent();
	RotationRoot = Cast<USceneComponent> ( GetOwner()->GetDefaultSubobjectByName(RotationRootComponentName) );
	if ( !RotationRoot )
	{
		UE_LOG ( LogActor, Warning, TEXT("UPlatformRotationComponent couldn't find rotation root with name: %s Falling back to root component."), *RotationRootComponentName.ToString());
		RotationRoot = GetOwner() -> GetRootComponent(); 
	}
	ensure ( RotationRoot ); 
}

void UPlatformRotationComponent::ResetRotation()
{
	RotationRoot -> SetRelativeRotation ( FRotator() ); 
}

