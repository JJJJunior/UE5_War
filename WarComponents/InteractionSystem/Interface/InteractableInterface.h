#pragma once

#include "CoreMinimal.h"
#include "Characters/Hero/WarHeroCharacter.h"
#include "UObject/Interface.h"
#include "InteractableInterface.generated.h"

UINTERFACE()
class UInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

class WAR_API IInteractableInterface
{
	GENERATED_BODY()

public:
	// 交互函数
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	void Interact();
	// 瞄准进入时
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	void OnBeginFocus();
	// 瞄准离开时
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	void OnEndFocus();
};
