// Fill out your copyright notice in the Description page of Project Settings.


#include "SCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ASCharacter::ASCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;
	GetMovementComponent()->GetNavAgentPropertiesRef().bCanJump = true;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	
	CameraComp->bUsePawnControlRotation;

	SprintSpeedMultiplier = 2.0f;

	ZoomedFOV = 65.0f;
	ZoomInterpSpeed = 1;
}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	DefaultFOV = CameraComp->FieldOfView;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	CurrentWeapon = GetWorld()->SpawnActor<ASWeapon>(StartWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator);

	if (CameraComp) {
		CameraComp->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, "head");
	}

	if (CurrentWeapon) {
		CurrentWeapon->SetOwner(this);
		CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);
	}
}

void ASCharacter::MoveForward(float Value)
{
	AddMovementInput(GetActorForwardVector() * Value);
}

void ASCharacter::MoveRight(float Value)
{
	AddMovementInput(GetActorRightVector() * Value);
}

void ASCharacter::CrouchFunction()
{
	if (Crouching) {
		UnCrouch();
		Crouching = false;
	}
	else if (!Crouching) {
		Crouch();
		Crouching = true;
	}
}

void ASCharacter::StartJump()
{
	Jump();
}

void ASCharacter::StartSprinting()
{
	if (!bWantsToZoom) {
		GetCharacterMovement()->MaxWalkSpeed *= SprintSpeedMultiplier;
		bWantsToZoom = false;
		sprinting = true;
	}
	
}

void ASCharacter::StopSprinting()
{
	GetCharacterMovement()->MaxWalkSpeed = 600;
	sprinting = false;
}

void ASCharacter::Fire()
{
	if (CurrentWeapon) {
		CurrentWeapon->Fire();
	}
}

void ASCharacter::BeginZoom()
{
	bWantsToZoom = true;
	if (CurrentWeapon) {
		CurrentWeapon->WeaponSpreadSet1();
	}
	if (CameraComp) {
		CameraComp->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, "FPSSocket");
	}
}

void ASCharacter::EndZoom()
{
	bWantsToZoom = false;
	if (CurrentWeapon) {
		CurrentWeapon->ResetWeaponSpread();
	}
	if (CameraComp) {
		CameraComp->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, "head");
	}
}

// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float TargetFOV = bWantsToZoom ? ZoomedFOV : DefaultFOV;

	float NewFOV = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime, ZoomInterpSpeed);

	CameraComp->SetFieldOfView(NewFOV);

	if (bWantsToZoom && sprinting) {
		StopSprinting();
	}

}

// Called to bind functionality to input
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacter::MoveRight);

	PlayerInputComponent->BindAxis("LookUp", this, &ASCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &ASCharacter::AddControllerYawInput);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASCharacter::CrouchFunction);

	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ASCharacter::StartJump);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ASCharacter::StartSprinting);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ASCharacter::StopSprinting);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASCharacter::Fire);

	PlayerInputComponent->BindAction("AimDownSights", IE_Pressed, this, &ASCharacter::BeginZoom);
	PlayerInputComponent->BindAction("AimDownSights", IE_Released, this, &ASCharacter::EndZoom);
}

FVector ASCharacter::GetPawnViewLocation() const
{
	if (CameraComp) {
		return CameraComp->GetComponentLocation();
	}
	return Super::GetPawnViewLocation();
}