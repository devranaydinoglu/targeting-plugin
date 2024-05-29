// Copyright 2023 devran All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TargetingComponent.generated.h"

class UCameraComponent;
enum ETraceTypeQuery;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTargetFoundDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTargetLostDelegate);

USTRUCT(BlueprintType)
struct FTargetData
{
	GENERATED_BODY()
	FTargetData() : Target(nullptr), Score(0.0f) {};

	UPROPERTY(BlueprintReadOnly, Category = "Targeting")
	AActor* Target;

	UPROPERTY(BlueprintReadOnly, Category = "Targeting")
	float Score;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SOFTTARGETINGSYSTEM_API UTargetingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties.
	UTargetingComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Player character reference.
	UPROPERTY(BlueprintReadWrite, Category = "Targeting")
	ACharacter* PlayerCharacter;

	// Player camera reference.
	UPROPERTY(BlueprintReadWrite, Category = "Targeting")
	UCameraComponent* PlayerCamera;

	// Handle for search timer.
	FTimerHandle SearchTimerHandle;

	// Used to start timer initially.
	bool bStartTimer;

	// Radius around player character to find targets in.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Targeting")
	float SearchRadius;

	// Time between searches.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Targeting")
	float SearchInterval;
	
	// Max horizontal angle of camera direction in which targets will be detected. Max should be the camera's FOV divided by 2.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "90.0"), Category = "Targeting")
	float MaxHorizontalCameraAngle;

	// Max vertical angle of camera direction in which targets will be detected. Max should be the camera's FOV divided by 3.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "60.0"), Category = "Targeting")
	float MaxVerticalCameraAngle;

	// Max horizontal angle of player direction in which targets will be detected.
	// This is the angle measured in relation to the forward vector, not the whole 360 degrees range.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "180.0"), Category = "Targeting")
	float MaxHorizontalPlayerHalfAngle;

	// Determines how much the target's angle to the camera direction affects target scores.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "1.0"), Category = "Targeting")
	float CameraDirectionMultiplier;

	// Determines how much the target's distance to the player affects target scores.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "1.0"), Category = "Targeting")
	float DistanceMultiplier;

	// Determines how much the target's angle to the player character direction affects target scores.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "1.0"), Category = "Targeting")
	float PlayerDirectionMultiplier;

	// Tag used to find targets.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Targeting")
	FName TargetTag;

	// Actor class used to find targets.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Targeting")
	TSubclassOf<AActor> TargetClass;

	// Trace channel used to detect targets.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Targeting")
	TEnumAsByte<ETraceTypeQuery> TargetTraceChannel;

	// Trace channel used to detect blocking actors.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Targeting")
	TEnumAsByte<ETraceTypeQuery> BlockingTraceChannel;

	// Target reference.
	UPROPERTY(BlueprintReadWrite, Category = "Targeting")
	AActor* Target;

	// All targets currently in range ranked by their score.
	UPROPERTY(BlueprintReadOnly, Category = "Targeting")
	TArray<FTargetData> RankedTargets;
	
	// Enable the printing of warnings and errors if any are encountered.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Targeting")
	bool bDebug;

	UPROPERTY(BlueprintAssignable)
	FTargetFoundDelegate OnTargetFound;

	UPROPERTY(BlueprintAssignable)
	FTargetLostDelegate OnTargetLost;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// Initialize component and activate targeting.
	UFUNCTION(BlueprintCallable, Category = "Targeting")
	void Initialize(ACharacter* InPlayerCharacter, UCameraComponent* InPlayerCamera);

	// Start or unpause search timer.
	UFUNCTION(BlueprintCallable, Category = "Targeting")
	void ActivateTargeting();

	// Pause search timer.
	UFUNCTION(BlueprintCallable, Category = "Targeting")
	void DeactivateTargeting();

	// Set optimal target as target.
	void SetTarget();

	// Check if actor is in player's vision.
	bool IsInVision(const AActor* Actor);

	// Find most optimal target.
	AActor* FindOptimalTarget(TArray<AActor*> TargetsArray);

	// Score target on each criteria.
	float ScoreTarget(const AActor* Target);

};
