
#pragma once

#include "CoreMinimal.h"
#include "AnimInstance/WarBaseAnimInstance.h"
#include "WarCharacterAnimInstance.generated.h"


class AWarCharacterBase;
class UCharacterMovementComponent;

UCLASS()
class WAR_API UWarCharacterAnimInstance : public UWarBaseAnimInstance
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AnimInstance")
	TObjectPtr<AWarCharacterBase> OwningCharacter;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AnimInstance")
	TObjectPtr<UCharacterMovementComponent> OwningMovementComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AnimInstance")
	float GroundSpeed;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AnimInstance")
	bool bHasAccelerated;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AnimInstance")
	bool bIsFalling;

public:
	virtual void NativeThreadSafeUpdateAnimation(float DeltaTime) override;
	virtual void NativeInitializeAnimation() override;
};
