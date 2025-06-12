#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InventoryBase.generated.h"


class UStaticMeshComponent;


UCLASS()
class WAR_API AInventoryBase : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY()
	TObjectPtr<USceneComponent> RootScene;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Inventory")
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

public:
	AInventoryBase();
};
