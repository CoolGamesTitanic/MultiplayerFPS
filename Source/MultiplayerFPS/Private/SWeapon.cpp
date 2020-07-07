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
#include "GameFramework/Character.h"

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
	
	WeaponSpread = DefaultWeaponSpread;
}

void ASWeapon::PlayFireEffect(FVector TracerEndPoint)
{
	FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

	if (FireAnimation) {
		MeshComp->PlayAnimation(FireAnimation, false);
	}
	else {
		if (MuzzleEffect) {
			UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
		}

		if (TracerEffect) {
			UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
			if (TracerComp) {
				TracerComp->SetVectorParameter(TracerTargetName, TracerEndPoint);
			}
		}
	}

	if ((FireAnimation && !FireAnimPlaysSound) || (!FireAnimation)) {
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSound, MuzzleLocation, 1.0f, 1.0f, 0.0f, SoundAttenuationSettings);
	}

	APawn* MyOwner = Cast<APawn>(GetOwner());
	if (MyOwner) {
		APlayerController* PC = Cast<APlayerController>(MyOwner->GetController());
		if (PC) {
			PC->ClientPlayCameraShake(FireCamShake);
		}
	}
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

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(MyOwner);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;
		QueryParams.bReturnPhysicalMaterial = true;

		FVector TraceStart = EyeLocation;

		FVector TraceEnd = TraceStart + (ShotDirection * 10000);

		TraceEnd.X += -WeaponSpread + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (WeaponSpread - -WeaponSpread)));
		TraceEnd.Y += -WeaponSpread + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (WeaponSpread - -WeaponSpread)));

		//Particle "target" paramater
		FVector TracerEndPoint = TraceEnd;

		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, QueryParams)) {
			//blocking hit process damage

			AActor* HitActor = Hit.GetActor();

			UGameplayStatics::ApplyPointDamage(HitActor, 20.0f, ShotDirection, Hit, MyOwner->GetInstigatorController(), this, DamageType);

			EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

			UParticleSystem* SelectedParticleEffect = nullptr;
			USoundBase* SelectedSoundEffect = nullptr;

			UMaterialInterface* SelectedDecal = nullptr;
			float SelectedDecalLifeSpan = 25;

			switch (SurfaceType)
			{
			case SURFACE_FLESHVULNERABLE:
			case SURFACE_FLESHDEFAULT:
				SelectedParticleEffect = FleshImpactEffect;
				SelectedSoundEffect = FleshImpactSound;
				break;
			default:
				SelectedParticleEffect = DefaultImpactEffect;
				SelectedSoundEffect = DefaultImpactSound;
				SelectedDecal = DefaultImpactDecal;
				SelectedDecalLifeSpan = DefaultDecalLifeSpan;
				break;
			}

			if (SelectedParticleEffect) {
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedParticleEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
			}
			if (SelectedSoundEffect) {
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), SelectedSoundEffect, Hit.ImpactPoint, 1, 1, 0, SoundAttenuationSettings);
			}
			if (SelectedDecal) {
				FRotator ActorRotation = Hit.GetActor()->GetActorRotation();
				UGameplayStatics::SpawnDecalAtLocation(GetWorld(), SelectedDecal, FVector(DecalSize, DecalSize, DecalSize), Hit.ImpactPoint, FRotator(ActorRotation.Vector().X - 90, ActorRotation.Vector().Y, ActorRotation.Vector().Z), SelectedDecalLifeSpan);
				UE_LOG(LogTemp, Warning, TEXT("%f"), FRotator(ActorRotation.Vector().X - 90));
			}
		}

		if (DebugWeaponsDrawing > 0) {
			DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 1.0f, 0, 1.0f);
		}

		PlayFireEffect(TracerEndPoint);
	}
}

void ASWeapon::WeaponSpreadSet1()
{
	WeaponSpread = 1;
}

void ASWeapon::ResetWeaponSpread()
{
	WeaponSpread = DefaultWeaponSpread;
}

