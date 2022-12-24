// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BaseAIWeapon.h"

#include "DrawDebugHelpers.h"
#include "AI/AICharacterController.h"
#include "Isolation/Isolation.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
ABaseAIWeapon::ABaseAIWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
	
	// Creating the skeletal meshes for our attachments and making sure that they do not cast shadows

	BarrelAttachment = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BarrelAttachment"));
	BarrelAttachment->SetupAttachment(RootComponent);

	MagazineAttachment = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MagazineAttachment"));
	MagazineAttachment->SetupAttachment(RootComponent);

	SightsAttachment = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SightsAttachment"));
	SightsAttachment->SetupAttachment(RootComponent);

	StockAttachment = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("StockAttachment"));
	StockAttachment->SetupAttachment(RootComponent);

	GripAttachment = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GripAttachment"));
	GripAttachment->SetupAttachment(RootComponent);
}

void ABaseAIWeapon::Fire()
{
	const AAICharacterController* ParentController = Cast<AAICharacterController>(GetParentActor()->GetInstigatorController());
	const AActor* TargetActor = ParentController->GetTargetActor();

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.bTraceComplex = true;
	QueryParams.bReturnPhysicalMaterial = true;
	
	if (bCanFire && RuntimeWeaponData.ClipSize > 0)
	{
		const int NumberOfShots = StaticWeaponData.bIsShotgun? StaticWeaponData.ShotgunPellets : 1;
		for (int i = 0; i < NumberOfShots; ++i)
		{
			FVector TraceStart = MeshComp->GetSocketLocation(StaticWeaponData.MuzzleLocation);
			FRotator TraceStartRotation = UKismetMathLibrary::FindLookAtRotation(MeshComp->GetSocketLocation(StaticWeaponData.MuzzleLocation), TargetActor->GetActorLocation());

			TraceStartRotation.Pitch += FMath::FRandRange(
	-((StaticWeaponData.WeaponPitchVariation + WeaponPitchModifier)),
	(StaticWeaponData.WeaponPitchVariation + WeaponPitchModifier));
			TraceStartRotation.Yaw += FMath::FRandRange(
				-((StaticWeaponData.WeaponYawVariation + WeaponYawModifier)),
				(StaticWeaponData.WeaponYawVariation + WeaponYawModifier));
			FVector TraceDirection = TraceStartRotation.Vector();
			FVector TraceEnd = TraceStart + (TraceDirection * (StaticWeaponData.bIsShotgun
														   ? StaticWeaponData.ShotgunRange
														   : StaticWeaponData.LengthMultiplier));

			if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ENEMYWEAPON_TRACE, QueryParams))
			{
				DrawDebugLine(GetWorld(), (StaticWeaponData.bHasAttachments
					              ? BarrelAttachment->GetSocketLocation(StaticWeaponData.MuzzleLocation)
					              : MeshComp->GetSocketLocation(StaticWeaponData.MuzzleLocation)), Hit.Location,
					FColor::Blue, false, 10.0f, 0.0f, 2.0f);

				UGameplayStatics::ApplyPointDamage(Hit.GetActor(), 10.0f, TraceDirection, Hit, GetInstigatorController(), this, DamageType);
				
			}
		}
	}	
}

void ABaseAIWeapon::SpawnAttachments()
{
	if (StaticWeaponData.bHasAttachments)
    {
        for (FName RowName : RuntimeWeaponData.WeaponAttachments)
        {
            // Going through each of our attachments and updating our static weapon data accordingly
            AttachmentData = StaticWeaponData.AttachmentsDataTable->FindRow<FAttachmentData>(RowName, RowName.ToString(), true);

            if (AttachmentData)
            {
                DamageModifier += AttachmentData->BaseDamageImpact;
                WeaponPitchModifier += AttachmentData->WeaponPitchVariationImpact;
                WeaponYawModifier += AttachmentData->WeaponYawVariationImpact;

                if (AttachmentData->AttachmentType == EAttachmentType::Barrel)
                {

                    BarrelAttachment->SetSkeletalMesh(AttachmentData->AttachmentMesh);
                    StaticWeaponData.MuzzleLocation = AttachmentData->MuzzleLocationOverride;
                    StaticWeaponData.ParticleSpawnLocation = AttachmentData->ParticleSpawnLocationOverride;
                    StaticWeaponData.bSilenced = AttachmentData->bSilenced;
                }
                else if (AttachmentData->AttachmentType == EAttachmentType::Magazine)
                {
                    MagazineAttachment->SetSkeletalMesh(AttachmentData->AttachmentMesh);
                    StaticWeaponData.FireSound = AttachmentData->FiringSoundOverride;
                    StaticWeaponData.SilencedSound = AttachmentData->SilencedFiringSoundOverride;
                    StaticWeaponData.RateOfFire = AttachmentData->FireRate;
                    StaticWeaponData.bAutomaticFire = AttachmentData->AutomaticFire;
                    StaticWeaponData.bIsShotgun = AttachmentData->bIsShotgun;
                    StaticWeaponData.ShotgunRange = AttachmentData->ShotgunRange;
                    StaticWeaponData.ShotgunPellets = AttachmentData->ShotgunPellets;
                }
                else if (AttachmentData->AttachmentType == EAttachmentType::Sights)
                {
                    SightsAttachment->SetSkeletalMesh(AttachmentData->AttachmentMesh);
                    StaticWeaponData.bAimingFOV = AttachmentData->bAimingFOV;
                    StaticWeaponData.AimingFOVChange = AttachmentData->AimingFOVChange;
                    StaticWeaponData.bIsScope = AttachmentData->bIsScope;
                    StaticWeaponData.ScopeMagnification = AttachmentData->ScopeMagnification;
                    StaticWeaponData.UnmagnifiedLFoV = AttachmentData->UnmagnifiedLFoV;
                }
                else if (AttachmentData->AttachmentType == EAttachmentType::Stock)
                {
                    StockAttachment->SetSkeletalMesh(AttachmentData->AttachmentMesh);
                }
                else if (AttachmentData->AttachmentType == EAttachmentType::Grip)
                {
                    GripAttachment->SetSkeletalMesh(AttachmentData->AttachmentMesh);
                }
            }
        }
    }
}

// Called when the game starts or when spawned
void ABaseAIWeapon::BeginPlay()
{
	Super::BeginPlay();

	StaticWeaponData = *WeaponDataTable->FindRow<FAIWeaponData>(FName(DataTableNameRef), FString(DataTableNameRef), true);
	
}

// Called every frame
void ABaseAIWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

