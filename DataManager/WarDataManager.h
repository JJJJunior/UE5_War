#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "War/DataManager/DynamicData/InventoryData.h"
#include "WarDataManager.generated.h"

UCLASS()
class WAR_API UWarDataManager : public UObject
{
	GENERATED_BODY()

public:
	static bool CreateWeapon(const UObject* WorldContextObject, const FName& TableID, const FGuid& PlayerID, FItemInBagData& ItemInBagData);
	static bool CreateArmor(const UObject* WorldContextObject, const FName& TableID, const FGuid& PlayerID, FItemInBagData& ItemInBagData);
	static bool CreateConsumable(const UObject* WorldContextObject, const FName& TableID, const FGuid& PlayerID, FItemInBagData& ItemInBagData);
	static bool CreateQuestItem(const UObject* WorldContextObject, const FName& TableID, const FGuid& PlayerID, FItemInBagData& ItemInBagData);
	static bool CreateSkill(const UObject* WorldContextObject, const FName& TableID, const FGuid& PlayerID, FItemInBagData& ItemInBagData);
};
