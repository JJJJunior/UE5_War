#pragma once

#include "CoreMinimal.h"
#include "GameplayTags.h"
#include "WarInputActionType.generated.h"


class UInputAction;
struct FGameplayTag;

USTRUCT(BlueprintType)
struct FWarInputActionType
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="War|InputData")
	TObjectPtr<UInputAction> InputAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="War|InputData")
	FGameplayTag GameplayTag;
	bool IsValid() const;
};

