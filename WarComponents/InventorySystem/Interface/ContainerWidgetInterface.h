#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ContainerWidgetInterface.generated.h"


class AInventoryBase;
struct FWarInventoryRow;
struct FInventoryInstanceData;

UINTERFACE()
class UContainerWidgetInterface : public UInterface
{
	GENERATED_BODY()
};


class WAR_API IContainerWidgetInterface
{
	GENERATED_BODY()

public:
	virtual void InitSlots() = 0;
	virtual void SyncSlots() = 0;
	virtual void RemoveItemFromSlot(const FGuid& InID) =0;
	virtual void ClearAllSlots() =0;
	virtual void AddItemToSlot(const FGuid& InID) =0;
};
