#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "War/DataManager/DynamicData/InventoryData.h"
#include "ContainerWidgetInterface.generated.h"


class AInventoryBase;
class UItemSlotWidget;

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
	virtual void RemoveItemFromSlot(const FItemInBagData& InBagData) =0;
	virtual void ClearAllSlots() =0;
	virtual void AddItemToSlot(const FItemInBagData& InBagData) =0;
	virtual int32 GetMaxSlots() const =0;
};
