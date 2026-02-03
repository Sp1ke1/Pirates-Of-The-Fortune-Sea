// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlatformRotationComponent.h"
#include "Components/BoxComponent.h"
#include "PlatformMassBoundsComponent.generated.h"


class UPlatformRotationComponent; 
class UMassComponent; 

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GDS_2025_API UPlatformMassBoundsComponent : public UBoxComponent
{
	GENERATED_BODY()

public:
	UPlatformMassBoundsComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
	UFUNCTION(BlueprintCallable) 
	void SetRotationComponent (UPlatformRotationComponent* InRotationComponent); 
	

protected: 
	UFUNCTION()
	void OnComponentBeginOverlapImpl ( UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult); 
	UFUNCTION()
	void OnComponentEndOverlapImpl (UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex );
	virtual void InitializeComponent() override;
	
	UPROPERTY(EditAnywhere, Category = "Setup")
	float MassToRotationMultiplier = 0.3f; 

	UPROPERTY(EditAnywhere, Category = "Setup")
	int	BoundsIndex = 0;

	UPROPERTY(EditAnywhere, Category = "Setup")
	bool bRotationSign = true; 
	
	UPROPERTY(EditAnywhere, Category = "Setup")
	TSubclassOf<UPlatformRotationComponent> RotationComponentClass = UActorComponent::StaticClass();

	UPROPERTY(EditAnywhere, Category = "Setup")
	TSubclassOf<UPlatformRotationComponent> MassComponentClass = UActorComponent::StaticClass();
	
	UPROPERTY(EditAnywhere, Category = "Setup")
	bool bAutoInitializeRotationComponent = true;
	
	UPROPERTY()
	TSet<UMassComponent*> MassControllersInBounds;
	
	UPROPERTY() 
	UPlatformRotationComponent* RotationComponent;
	
	float TotalMassInBounds; 
};
