#include "WarInteractionComponent.h"
#include "Camera/CameraComponent.h"
#include "War/WarComponents/InteractionSystem/Interface/InteractableInterface.h"
#include "Characters/Hero/WarHeroCharacter.h"
#include "WarComponents/InteractionSystem/ProjectCollision.h"


UWarInteractionComponent::UWarInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UWarInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	CachedWarHeroCharacter = Cast<AWarHeroCharacter>(GetOwner());
	checkf(CachedWarHeroCharacter, TEXT("CachedWarHeroCharacter is null"));
}


void UWarInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	FHitResult Hit;
	FVector Start = CachedWarHeroCharacter->GetActorLocation();
	FVector End = Start + (CachedWarHeroCharacter->GetFollowCamera()->GetForwardVector() * InteractorRange);

	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(CachedWarHeroCharacter);

	// 使用球形检测，增加容错范围
	bool bHit = GetWorld()->SweepSingleByChannel(
		Hit,
		Start,
		End,
		FQuat::Identity,
		ECC_PlayerInteraction, //使用宏定义通道
		FCollisionShape::MakeSphere(InteractionRadius),
		TraceParams
	);

	if (bHit && Hit.GetActor()->Implements<UInteractableInterface>())
	{
		if (CurrentInteractable != Hit.GetActor())
		{
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

void UWarInteractionComponent::TryInteract()
{
	// FHitResult HitResult;
	// // 执行Trace检测
	//
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
	// }
}
