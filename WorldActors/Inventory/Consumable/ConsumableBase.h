
#pragma once

#include "CoreMinimal.h"
#include "WorldActors/Inventory/InventoryBase.h"
#include "ConsumableBase.generated.h"

UCLASS()
class WAR_API AConsumableBase : public AInventoryBase
{
	GENERATED_BODY()

public:
	AConsumableBase();

protected:
	virtual void BeginPlay() override;
};
