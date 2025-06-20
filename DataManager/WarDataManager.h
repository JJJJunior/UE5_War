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
	static TOptional<FItemInBagData> CreateWeapon(const UObject* WorldContextObject, const FName& TableID, const FGuid& PlayerID);
	static TOptional<FItemInBagData> CreateArmor(const UObject* WorldContextObject, const FName& TableID, const FGuid& PlayerID);
	static TOptional<FItemInBagData> CreateConsumable(const UObject* WorldContextObject, const FName& TableID, const FGuid& PlayerID);
	static TOptional<FItemInBagData> CreateQuestItem(const UObject* WorldContextObject, const FName& TableID, const FGuid& PlayerID);
	static TOptional<FItemInBagData> CreateSkill(const UObject* WorldContextObject, const FName& TableID, const FGuid& PlayerID);
};
