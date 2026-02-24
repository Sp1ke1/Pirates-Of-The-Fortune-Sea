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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Logic | Team")
	int32 TeamIndex;

	/** Sets TeamIndex and auto-computes TeamZoneTag and EnemyBaseTag */
	UFUNCTION(BlueprintCallable, Category = "AI Logic | Team")
	void SetTeamIndex(int32 NewTeamIndex);

	/** Tag of the pickup zone actor (auto-set from TeamIndex) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Logic | Team")
	FName TeamZoneTag;

	/** Tag of the enemy base (auto-set from TeamIndex) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Logic | Team")
	FName EnemyBaseTag;

	/** Tag to search for items to pick up */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Logic")
	FName TargetItemTag;

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
	AActor* FindNearestItemInZone();
	bool IsActorInTeamZone(AActor* Actor) const;
	bool MoveToActor(AActor* Target);
	FVector GetActorPhysicalLocation(AActor* Target);
	
	// Helper to find interface
	class IPickupInterface* GetPickupComponent() const;

	bool IsTargetReached(AActor* Target);
};
