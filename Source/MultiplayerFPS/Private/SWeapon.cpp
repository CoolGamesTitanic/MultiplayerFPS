// Fill out your copyright notice in the Description page of Project Settings.


#include "SWeapon.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "HAL/IConsoleManager.h"
#include "Sound/SoundAttenuation.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "MultiplayerFPS/MultiplayerFPS.h"
#include "Math/UnrealMathUtility.h"

static int32 DebugWeaponsDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(
	TEXT("FPS.DebugWeapons"),
	DebugWeaponsDrawing,
	TEXT("Draw Debug Lines for Weapon"),
	ECVF_Cheat);

// Sets default values
ASWeapon::ASWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "MozzleFlashSocket";
	TracerTargetName = "Target";
}

// Called when the game starts or when spawned
void ASWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASWeapon::PlayFireEffect(FVector TracerEndPoint)
{
	if (MuzzleEffect) {
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}

	if (TracerEffect) {
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
		if (TracerComp) {
			TracerComp->SetVectorParameter(TracerTargetName, TracerEndPoint);
		}
	}

	APawn* MyOwner = Cast<APawn>(GetOwner());
	if (MyOwner) {
		APlayerController* PC = Cast<APlayerController>(MyOwner->GetController());
		if (PC) {
			PC->ClientPlayCameraShake(FireCamShake);
		}
	}

	FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

	UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSound, MuzzleLocation, 1.0f, 1.0f, 0.0f, SoundAttenuationSettings);
}

// Called every frame
void ASWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASWeapon::Fire()
{
	//trace the world from pawn eyes to crosshair location
	AActor* MyOwner = GetOwner();

	FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

	if (MyOwner) {
		FVector EyeLocation;
		FRotator EyeRotation;
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FVector ShotDirection = EyeRotation.Vector();

		ShotDirection.X += -WeaponSpread + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (WeaponSpread - -WeaponSpread)));

		FVector TraceEnd = MuzzleLocation + (ShotDirection * 10000);

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(MyOwner);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;
		QueryParams.bReturnPhysicalMaterial = true;

		//Particle "target" paramater
		FVector TracerEndPoint = TraceEnd;

		FVector TraceStart = EyeLocation;

		TraceStart.X += -WeaponSpread + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (WeaponSpread - -WeaponSpread)));
		TraceStart.Y += -WeaponSpread + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (WeaponSpread - -WeaponSpread)));

		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, QueryParams)) {
			//blocking hit process damage

			AActor* HitActor = Hit.GetActor();

			UGameplayStatics::ApplyPointDamage(HitActor, 20.0f, ShotDirection, Hit, MyOwner->GetInstigatorController(), this, DamageType);

			EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

			UParticleSystem* SelectedParticleEffect = nullptr;
			USoundBase* SelectedSoundEffect = nullptr;

			switch (SurfaceType)
			{
			case SurfaceType1:
			case SurfaceType2:
				
				SelectedParticleEffect = FleshImpactEffect;
				SelectedSoundEffect = FleshImpactSound;
				break;
			default:
				SelectedParticleEffect = DefaultImpactEffect;
				SelectedSoundEffect = DefaultImpactSound;
				UE_LOG(LogTemp, Warning, TEXT("kind of working"));
				break;
			}

			if (SelectedParticleEffect) {
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedParticleEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), SelectedSoundEffect, Hit.ImpactPoint, 1, 1, 0, SoundAttenuationSettings);
			}
		}

		if (DebugWeaponsDrawing > 0) {
			DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 1.0f, 0, 1.0f);
		}

		PlayFireEffect(TracerEndPoint);
	}
}

