// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SWeapon.h"
#include "SCharacter.generated.h"

class UCameraComponent;
class USkeletalMeshComponent;

UCLASS()
class MULTIPLAYERFPS_API ASCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float Value);
	void MoveRight(float Value);

	void CrouchFunction();

	void StartJump();

	void StartSprinting();
	void StopSprinting();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	bool Sprinting = false;

	ASWeapon* CurrentWeapon;

	void Fire();

	UPROPERTY(EditDefaultsOnly, Category = "Player")
		TSubclassOf<ASWeapon> StartWeaponClass;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
		FName WeaponAttachSocketName;

	bool Crouching = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float SprintSpeedMultiplier;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual FVector GetPawnViewLocation() const override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UCameraComponent* CameraComp;

};