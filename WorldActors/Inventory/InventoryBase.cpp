#include "InventoryBase.h"

#include "Components/SphereComponent.h"


AInventoryBase::AInventoryBase()
{
	PrimaryActorTick.bCanEverTick = false;
	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = RootScene;
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(RootComponent);
	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(StaticMeshComponent);
}

void AInventoryBase::BeginPlay()
{
	Super::BeginPlay();
	InteractionSphere->SetHiddenInGame(false);
}

void AInventoryBase::DisableInteractionSphere() const
{
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}


void AInventoryBase::Interact_Implementation(AWarHeroCharacter* Interactor)
{
	// 例如：拾取物品
	UE_LOG(LogTemp, Warning, TEXT("拾取物品...."));
}

void AInventoryBase::OnBeginFocus_Implementation()
{
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, TEXT("OnBeginFocus_Implementation...."));
}

void AInventoryBase::OnEndFocus_Implementation()
{
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, TEXT("OnEndFocus_Implementation...."));
}
