#include "WarInteractionComponent.h"
#include "WarPlayerController.h"
#include "War/WarComponents/InteractionSystem/Interface/InteractableInterface.h"
#include "Characters/Hero/WarHeroCharacter.h"
#include "CollisionShape.h"
#include "ProjectCollision.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"


UWarInteractionComponent::UWarInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	InteractionSphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
}


void UWarInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	CachedWarHeroCharacter = Cast<AWarHeroCharacter>(GetOwner());
	CachedWarPlayerController = Cast<AWarPlayerController>(CachedWarHeroCharacter->Controller);
	check(CachedWarHeroCharacter);
	check(CachedWarPlayerController);

	InteractionSphereComponent->AttachToComponent(CachedWarHeroCharacter->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	InteractionSphereComponent->SetCollisionObjectType(InteractionComponent);
	InteractionSphereComponent->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::BeginOnOverlap);
	InteractionSphereComponent->OnComponentEndOverlap.AddDynamic(this, &ThisClass::EndOnOverlap);
	InteractionSphereComponent->SetHiddenInGame(false);

	TraceParams.AddIgnoredActor(CachedWarHeroCharacter);
	TraceParams.bTraceComplex = true; // 是否使用复杂碰撞
	TraceParams.bReturnPhysicalMaterial = false; //是否返回物理材质
}


void UWarInteractionComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	CrosshairTrace();
}


//用于准星碰撞检测
void UWarInteractionComponent::CrosshairTrace()
{
	if (!CachedWarPlayerController) return;
	// 获取屏幕中心点
	int32 ViewportSizeX, ViewportSizeY;
	CachedWarPlayerController->GetViewportSize(ViewportSizeX, ViewportSizeY);
	FVector2D ScreenCenter(ViewportSizeX / 2.f, ViewportSizeY / 2.f);
	FVector WorldLocation; // 射线起点
	FVector WorldDirection; // 射线方向

	// 转换屏幕点到世界方向
	bool bSuccess = CachedWarPlayerController->DeprojectScreenPositionToWorld(ScreenCenter.X, ScreenCenter.Y, WorldLocation, WorldDirection);

	if (bSuccess)
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

	// //主动检测，拾取、开门、按钮
	// if (AActor* HitActor = HitResult.GetActor())
	// {
	// 	if (UWarInteractionComponent* InteractionComp = HitActor->FindComponentByClass<UWarInteractionComponent>())
	// 	{
	// 		switch (InteractionComp->InteractionType)
	// 		{
	// 		case EInteractionType::Pickup:
	// 			HandlePickup(HitActor);
	// 			break;
	// 		case EInteractionType::OpenDoor:
	// 			HandleOpenDoor(HitActor);
	// 			break;
	// 		case EInteractionType::Equip:
	// 			HandleEquip(HitActor);
	// 			break;
	// 		case EInteractionType::Talk:
	// 			HandleTalk(HitActor);
	// 			break;
	// 		default:
	// 			break;
	// 		}
	// 	}
}

void UWarInteractionComponent::BeginOnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, FString::Printf(TEXT("%s BeginOnOverlap!!!"), *OtherActor->GetName()));
}

void UWarInteractionComponent::EndOnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Purple, FString::Printf(TEXT("%s EndOnOverlap!!!"), *OtherActor->GetName()));
}
