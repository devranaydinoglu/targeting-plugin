// Copyright 2023 devran All Rights Reserved.

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
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Targeting")
	float SearchRadius;

	// Time between searches
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Targeting")
	float SearchInterval;
	
	// Max horizontal angle of camera direction in which targets will be detected
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Targeting")
	float MaxHorizontalVisionAngle;

	// Max vertical angle of camera direction in which targets will be detected
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Targeting")
	float MaxVerticalVisionAngle;

	// Determines how much the target's angle to the camera direction affects target scores
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "1.0"), Category = "Targeting")
	float CameraDirectionMultiplier;

	// Determines how much the target's distance to the player affects target scores
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "1.0"), Category = "Targeting")
	float DistanceMultiplier;

	// Determines how much the target's angle to the player character direction affects target scores
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "1.0"), Category = "Targeting")
	float PlayerDirectionMultiplier;

	// Tag used to find targets
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Targeting")
	FName TargetTag;

	// Trace channel used to detect targets
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Targeting")
	TEnumAsByte<ETraceTypeQuery> TargetTraceChannel;

	// Trace channel used to detect blocking actors
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Targeting")
	TEnumAsByte<ETraceTypeQuery> BlockingTraceChannel;

	// Target reference
	UPROPERTY(BlueprintReadOnly, Category = "Targeting")
	AActor* Target;

	// Handle for search timer
	FTimerHandle SearchTimerHandle;

	// Player character reference
	UPROPERTY(BlueprintReadOnly, Category = "Targeting")
	ACharacter* PlayerCharacter;

	// Player camera reference
	UPROPERTY(BlueprintReadOnly, Category = "Targeting")
	UCameraComponent* PlayerCamera;

	// Used to start timer initially
	bool bStartTimer;

	// Enable the printing of warnings and errors if any are encountered
	UPROPERTY(EditAnywhere, Category = "Targeting")
	bool bDebug;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// Initialize component and activate targeting
	UFUNCTION(BlueprintCallable, Category = "Targeting")
	void Initialize(ACharacter* InPlayerCharacter, UCameraComponent* InPlayerCamera);

	// Start or unpause search timer
	UFUNCTION(BlueprintCallable, Category = "Targeting")
	void ActivateTargeting();

	// Pause search timer
	UFUNCTION(BlueprintCallable, Category = "Targeting")
	void DeactivateTargeting();

	// Set optimal target as target
	void SetTarget();

	// Check if actor is in player's vision
	bool IsInVision(const AActor* Actor);

	// Find most optimal target
	AActor* FindOptimalTarget(TArray<AActor*> TargetsArray);

	// Score target on each criteria
	float ScoreTarget(const AActor* Target);

};
