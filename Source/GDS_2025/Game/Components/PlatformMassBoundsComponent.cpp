// Fill out your copyright notice in the Description page of Project Settings.


#include "PlatformMassBoundsComponent.h"

#include "MassComponent.h"


// Sets default values for this component's properties
UPlatformMassBoundsComponent::UPlatformMassBoundsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	RotationComponentClass = UPlatformRotationComponent::StaticClass();
	MassComponentClass = UMassComponent::StaticClass();
}

void UPlatformMassBoundsComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPlatformMassBoundsComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                 FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if ( TotalMassInBounds > 0 )
	{
		const float DeltaAngle = RotationComponent->GetRotationSpeed() * MassToRotationMultiplier * TotalMassInBounds * bRotationSign * DeltaTime;
		RotationComponent -> AddDeltaRotation( DeltaAngle ); 
	}
}

void UPlatformMassBoundsComponent::SetRotationComponent(UPlatformRotationComponent* InRotationComponent)
{
	RotationComponent = InRotationComponent;
}

void UPlatformMassBoundsComponent::OnComponentBeginOverlapImpl(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	const FHitResult& SweepResult)
{
	UMassComponent* MassComponent = Cast<UMassComponent> ( OtherActor->GetComponentByClass(MassComponentClass) ); 
	if ( !MassComponent )
	{
		return; 
	}
	if ( MassControllersInBounds . Add ( MassComponent ).IsValidId() )
	{
		TotalMassInBounds += MassComponent->GetMass(); 
	}
}

void UPlatformMassBoundsComponent::OnComponentEndOverlapImpl(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UMassComponent* MassComponent = Cast<UMassComponent> ( OtherActor->GetComponentByClass(MassComponentClass) ); 
	if ( !MassComponent )
	{
		return; 
	}
	if ( MassControllersInBounds . Remove ( MassComponent ) > 0 )
	{
		TotalMassInBounds -= MassComponent->GetMass();
	}
}

void UPlatformMassBoundsComponent::InitializeComponent()
{
	Super::InitializeComponent();
	if ( bAutoInitializeRotationComponent )
	{
		RotationComponent = Cast<UPlatformRotationComponent> ( GetOwner() -> GetComponentByClass(RotationComponentClass) );
		ensure (RotationComponent != nullptr); 
	}
	OnComponentBeginOverlap.AddDynamic( this, &ThisClass::OnComponentBeginOverlapImpl );
	OnComponentEndOverlap.AddDynamic( this, &ThisClass::OnComponentEndOverlapImpl );
}

