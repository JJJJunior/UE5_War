#include "InventoryBase.h"
#include "Components/SphereComponent.h"
#include "War/WarComponents/InteractionSystem/ProjectCollision.h"


AInventoryBase::AInventoryBase()
{
	PrimaryActorTick.bCanEverTick = false;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = RootScene;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(RootComponent);
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	InteractorSphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("InteractorSphereComponent"));
	InteractorSphereComponent->SetupAttachment(GetRootComponent());
	InteractorSphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractorSphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractorSphereComponent->SetCollisionResponseToChannel(ECC_PlayerInteraction, ECR_Block); // 使用宏定义GameTraceChannel1
}

void AInventoryBase::Interact_Implementation(AWarHeroCharacter* Interactor)
{
	// 例如：拾取物品
	UE_LOG(LogTemp, Warning, TEXT("拾取物品...."));
}

void AInventoryBase::OnBeginFocus_Implementation()
{
	// 例如：显示高亮
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, TEXT("老子是一瓶药水。"));
}

void AInventoryBase::OnEndFocus_Implementation()
{
	// 例如：取消高亮
}

FString AInventoryBase::GetInteractText_Implementation() const
{
	return FString(TEXT("Press E to Interact"));
}
