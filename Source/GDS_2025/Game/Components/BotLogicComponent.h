#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BotLogicComponent.generated.h"

UENUM(BlueprintType)
enum class EBotState : uint8
{
	Idle,
	MovingToItem,
	CarryingItem,
	MovingToBase
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GDS_2025_API UBotLogicComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBotLogicComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Tag to search for items to pick up */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Logic")
	FName TargetItemTag;

	/** Tag to search for base to drop items */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Logic")
	FName TargetBaseTag;

	/** Distance to interact with targets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Logic")
	float InteractionRange;

	/** Time to wait between actions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Logic")
	float WaitTime;

	/** Enable debug drawing for navigation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Logic")
	bool bDebugDraw = false;

private:
	EBotState CurrentState;
	
	UPROPERTY()
	AActor* CurrentTargetActor;

	UPROPERTY()
	AActor* CarriedItem;

	float WaitTimer;

	void UpdateLogic(float DeltaTime);
	AActor* FindNearestActorWithTag(FName Tag);
	AActor* FindRandomActorWithTag(FName Tag);
	void MoveToActor(AActor* Target);
	
	// Helper to find interface
	class IPickupInterface* GetPickupComponent() const;

	bool IsTargetReached(AActor* Target);
};
