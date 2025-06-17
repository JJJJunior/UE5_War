
#include "WarWeaponBase.h"
#include "Components/BoxComponent.h"

AWarWeaponBase::AWarWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;

	WeaponCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("WeaponCollisionBox"));
	WeaponCollisionBox->SetupAttachment(StaticMeshComponent);
	WeaponCollisionBox->SetBoxExtent(FVector(10.0f, 10.0f, 10.0f));
	WeaponCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

UBoxComponent* AWarWeaponBase::GetWeaponCollisionBox() const
{
	return WeaponCollisionBox;
}
