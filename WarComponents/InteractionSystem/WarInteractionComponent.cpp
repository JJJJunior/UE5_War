#include "WarInteractionComponent.h"
#include "WarPlayerController.h"
#include "War/WarComponents/InteractionSystem/Interface/InteractableInterface.h"
#include "Characters/Hero/WarHeroCharacter.h"
#include "CollisionShape.h"
#include "ProjectCollision.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "GameInstance/WarGameInstanceSubSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Tools/MyLog.h"
#include "WarComponents/InventorySystem/WarInventoryComponent.h"
#include "WarComponents/PersistentSystem/WarPersistentSystem.h"
#include "WorldActors/Inventory/InventoryBase.h"


UWarInteractionComponent::UWarInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	InteractionSphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
}


void UWarInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	AWarHeroCharacter* Character = Cast<AWarHeroCharacter>(GetOwner());
	if (!Character) return;
	AWarPlayerController* PlayerController = Cast<AWarPlayerController>(Character->GetController());
	if (!PlayerController) return;

	InteractionSphereComponent->AttachToComponent(Character->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	InteractionSphereComponent->SetCollisionObjectType(InteractionComponent);
	InteractionSphereComponent->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::BeginOnOverlap);
	InteractionSphereComponent->OnComponentEndOverlap.AddDynamic(this, &ThisClass::EndOnOverlap);

	TraceParams.AddIgnoredActor(PlayerController);
	TraceParams.bTraceComplex = true; // 是否使用复杂碰撞
	TraceParams.bReturnPhysicalMaterial = false; //是否返回物理材质
}


void UWarInteractionComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
}


//用于准星碰撞检测
void UWarInteractionComponent::CrosshairTrace()
{
	AWarHeroCharacter* Character = Cast<AWarHeroCharacter>(GetOwner());
	if (!Character) return;
	AWarPlayerController* PlayerController = Cast<AWarPlayerController>(Character->GetController());
	if (!PlayerController) return;

	// 获取屏幕中心点
	int32 ViewportSizeX, ViewportSizeY;
	PlayerController->GetViewportSize(ViewportSizeX, ViewportSizeY);
	FVector2D ScreenCenter(ViewportSizeX / 2.f, ViewportSizeY / 2.f);
	FVector WorldLocation; // 射线起点
	FVector WorldDirection; // 射线方向

	// 转换屏幕点到世界方向
	if (PlayerController->DeprojectScreenPositionToWorld(ScreenCenter.X, ScreenCenter.Y, WorldLocation, WorldDirection))
	{
		FVector TraceEnd = WorldLocation + (WorldDirection * CrosshairTraceDistance);
		FHitResult Hit;

		bool bHit = GetWorld()->LineTraceSingleByChannel(
			Hit,
			WorldLocation,
			TraceEnd,
			ECC_Visibility,
			TraceParams
		);

		if (bHit && Hit.GetActor()->Implements<UInteractableInterface>())
		{
			if (CurrentInteractable != Hit.GetActor())
			{
				// DrawDebugLine(GetWorld(), WorldLocation, TraceEnd, bHit ? FColor::Green : FColor::Red, false, 1.0f, 0, 1.0f);
				// 离开之前的交互对象
				if (CurrentInteractable)
				{
					IInteractableInterface::Execute_OnEndFocus(CurrentInteractable);
				}
				// 进入新的交互对象
				CurrentInteractable = Hit.GetActor();
				IInteractableInterface::Execute_OnBeginFocus(CurrentInteractable);
			}
		}
		else
		{
			// 没有命中，清除当前交互对象
			if (CurrentInteractable)
			{
				IInteractableInterface::Execute_OnEndFocus(CurrentInteractable);
				CurrentInteractable = nullptr;
			}
		}
	}
}


void UWarInteractionComponent::BeginOnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AInventoryBase* Inventory = Cast<AInventoryBase>(OtherActor))
	{
		if (!Inventory->GetTableRowID().IsValid())
		{
			print_err(TEXT("Inventory Table Row ID is null"));
			return;
		}
		AWarHeroCharacter* Character = Cast<AWarHeroCharacter>(GetOwner());
		if (!Character) return;
		if (Character->GetWarInventoryComponent()->GenerateItemToBagAndSaved(Inventory->GetTableRowID()))
		{
			print(TEXT("发生了Overlap获取:%s"), *Inventory->GetTableRowID().ToString())
			UWarGameInstanceSubSystem* Subsystem = UGameplayStatics::GetGameInstance(this)->GetSubsystem<UWarGameInstanceSubSystem>();
			UWarPersistentSystem* PersistentSystem = Subsystem->GetWarPersistentSystem();
			if (PersistentSystem->MarkAsDestroyed(Inventory->GetPersistentID()))
			{
				Inventory->Destroy();
				//标记删除，存档用
				print(TEXT("销毁物品：%s 地址：%p"), *GetName(), this);
			}
		}
	}
}

void UWarInteractionComponent::EndOnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}
