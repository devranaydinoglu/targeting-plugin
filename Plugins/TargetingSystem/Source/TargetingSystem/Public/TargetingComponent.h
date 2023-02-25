// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TargetingComponent.generated.h"

class UCameraComponent;
enum ETraceTypeQuery;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TARGETINGSYSTEM_API UTargetingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTargetingComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Radius around player character to find targets in
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float SearchRadius;

	// Time between searches
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float SearchInterval;
	
	// Max horizontal angle of camera direction in which targets will be detected
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float MaxHorizontalVisionAngle;

	// Max vertical angle of camera direction in which targets will be detected
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float MaxVerticalVisionAngle;

	// Determines how much the target's angle to the camera direction affects target scores
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CameraDirectionMultiplier;

	// Determines how much the target's distance to the player affects target scores
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DistanceMultiplier;

	// Determines how much the target's angle to the player character direction affects target scores
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PlayerDirectionMultiplier;

	// Tag used to find targets
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FName TargetTag;

	// Trace channel used to detect targets
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TEnumAsByte<ETraceTypeQuery> TargetTraceChannel;

	// Trace channel used to detect blocking actors
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TEnumAsByte<ETraceTypeQuery> BlockingTraceChannel;

	// Target reference
	UPROPERTY(BlueprintReadOnly)
	AActor* Target;

	// Handle for search timer
	FTimerHandle SearchTimerHandle;

	// Player character reference
	UPROPERTY(BlueprintReadOnly)
	ACharacter* PlayerCharacter;

	// Player camera reference
	UPROPERTY(BlueprintReadOnly)
	UCameraComponent* PlayerCamera;

	// Used to start timer initially
	bool bStartTimer;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// Initialize component
	UFUNCTION(BlueprintCallable)
	void Initialize(ACharacter* Character, UCameraComponent* Camera);

	// Start or unpause search timer
	UFUNCTION(BlueprintCallable)
	void ActivateTargeting();

	// Pause search timer
	UFUNCTION(BlueprintCallable)
	void DeactivateTargeting();

	// Set optimal target as target
	void SetTarget();

	// Check if actor is in player's vision
	bool IsInVision(AActor* Actor);

	// Find most optimal target
	AActor* FindOptimalTarget(TArray<AActor*> TargetsArray);

	// Score target on each criteria
	float ScoreTarget(AActor* Target);

};
