#include "InventoryBase.h"
#include "Components/SphereComponent.h"
#include "WarComponents/PersistentSystem/WarPersistentSystem.h"


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
	//不要乱删，否则无法destory
	Super::BeginPlay();

	//初始化ID
	UWarPersistentSystem::GeneratorPersistentID(this);
}


void AInventoryBase::DisableInteractionSphere() const
{
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}


void AInventoryBase::Interact_Implementation()
{
	// 例如：拾取物品
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("拾取物品....."));
}


//鼠标指向做事
void AInventoryBase::OnBeginFocus_Implementation()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("OnBeginFocus_Implementation....."));
}

//鼠标离开做事
void AInventoryBase::OnEndFocus_Implementation()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("OnEndFocus_Implementation....."));
}


void AInventoryBase::SaveActorData(FMemoryWriter& MemoryWriter) const
{
}

void AInventoryBase::LoadActorData(FMemoryReader& MemoryReader) const
{
}
