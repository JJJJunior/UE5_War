#pragma once

#include "CoreMinimal.h"
#include "Templates/EnableIf.h"
#include "War/DataManager/DynamicData/InventoryInstanceData.h"
#include "WarPersistentSystem.generated.h"


class UWarDataManager;

USTRUCT(Blueprintable)
struct FWarSaveGameData
{
	GENERATED_BODY()

	UPROPERTY()
	FGuid ActorGuid;
	UPROPERTY()
	FTransform ActorTransform;
	UPROPERTY()
	TArray<uint8> ActorData; //序列化
};


UCLASS()
class WAR_API UWarPersistentSystem : public UObject
{
	GENERATED_BODY()

protected:
	UPROPERTY()
	TArray<FWarSaveGameData> GameSavedActors;

	// 静态存储容器
	TMap<FGuid, FInventoryItemData> GameSavedInventories;
	TMap<FGuid, FWeaponData> GameSavedWeapons;
	TMap<FGuid, FArmorData> GameSavedArmors;
	TMap<FGuid, FQuestItemData> GameSavedQuestItems;
	TMap<FGuid, FSkillData> GameSavedSkills;
	TMap<FGuid, FConsumableData> GameSavedConsumables;

	// 检查ID是否存在
	bool IsInstanceIDExist(const FGuid& ID) const;

public:
	// 静态方法：操作当前实例的容器
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	static bool AddInventoryToDB(const UObject* WorldContextObject, const FInventoryItemData& ItemData);

	UWarPersistentSystem();
	UFUNCTION(BlueprintCallable)
	void SaveGame();
	UFUNCTION(BlueprintCallable)
	void LoadGame();
	FORCEINLINE static FGuid SetPersistentActorGuid() { return FGuid::NewGuid(); }

	static void CheckInventoriesInDB(const UObject* WorldContextObject);
};
