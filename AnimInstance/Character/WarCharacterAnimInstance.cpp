#include "WarCharacterAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "War/Characters/WarCharacterBase.h"

void UWarCharacterAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaTime)
{
	if (!OwningCharacter || !OwningMovementComponent)
	{
		return;
	}
	GroundSpeed = OwningCharacter->GetVelocity().Size2D();
	bHasAccelerated = OwningMovementComponent->GetCurrentAcceleration().SizeSquared2D() > 0.f;
	bIsFalling = OwningMovementComponent->IsFalling();
}

void UWarCharacterAnimInstance::NativeInitializeAnimation()
{
	OwningCharacter = Cast<AWarCharacterBase>(TryGetPawnOwner());
	if (OwningCharacter)
	{
		OwningMovementComponent = OwningCharacter->GetCharacterMovement();
	}
}
