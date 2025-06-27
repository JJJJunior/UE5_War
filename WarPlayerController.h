#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "WarPlayerController.generated.h"

class UInputMappingContext;
class UWarInventorySlot;

UCLASS(abstract)
class AWarPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category ="Input", meta = (AllowPrivateAccess = "true"))
	TArray<UInputMappingContext*> DefaultMappingContexts;
	virtual void SetupInputComponent() override;

public:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category="PlayerControllerNotify")
	void RefreshInputMode(bool bInventoryVisible);
};
