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
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	InteractionSphere->SetGenerateOverlapEvents(false);
}

void AInventoryBase::BeginPlay()
{
	//不要乱删，否则无法destory
	Super::BeginPlay();

	//初始化ID
	UWarPersistentSystem::GeneratorPersistentID(this);
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
	FArchive& Ar = MemoryWriter;
	//保存物品状态
	uint8 StateAsByte = static_cast<uint8>(CurrentState);
	Ar << StateAsByte;
}

void AInventoryBase::LoadActorData(FMemoryReader& MemoryReader)
{
	FArchive& Ar = MemoryReader;

	uint8 StateAsByte = 0;
	Ar << StateAsByte;

	EInventoryWorldState LoadedState = static_cast<EInventoryWorldState>(StateAsByte);
	SetWorldState(LoadedState); // 会自动设置碰撞行为
}

void AInventoryBase::SetWorldState(const EInventoryWorldState& NewState)
{
	CurrentState = NewState;
	UpdateInteractionCollision();
}

void AInventoryBase::UpdateInteractionCollision() const
{
	if (!InteractionSphere) return;
	switch (CurrentState)
	{
	case EInventoryWorldState::Equipped:
	case EInventoryWorldState::None:
		InteractionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		InteractionSphere->SetGenerateOverlapEvents(false);
		break;
	case EInventoryWorldState::WorldDropped:
		InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		InteractionSphere->SetGenerateOverlapEvents(true);
		break;
	}
}
