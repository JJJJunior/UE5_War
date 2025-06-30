#pragma once

#include "CoreMinimal.h"
#include "DataManager/DynamicData/InventoryData.h"
#include "Templates/EnableIf.h"
#include "WarPersistentSystem.generated.h"


class UWarDataManager;
class AInventoryBase;

UCLASS()
class WAR_API UWarPersistentSystem : public UObject
{
	GENERATED_BODY()

protected:
	void InsertInventory(const FInventoryItemInDB& InventoryItemInDB) const;

public:
	FString SavedGameDataFieldName;
	FString SavedGameDataDBPath;
	TSharedPtr<FJsonObject> SavedGameDataJsonDB;
	FString InventoryFieldName;
	FString InventoryDBPath;
	TSharedPtr<FJsonObject> InventoryJsonDB;


	static bool SaveJsonToFile(const FString& JsonSavePath, const TSharedPtr<FJsonObject>& JsonObject);
	static bool InitJsonDB(const FString& JsonSavePath, TSharedPtr<FJsonObject>& OutJsonObject, const FString& DefaultFieldName, bool bInitAsArray);


	void InitSaveActors();
	void InitInventories();

	void InsertSavedActor(const FWarSaveGameData& SaveGameData) const;
	void UpdateSavedActor(const FWarSaveGameData& SaveGameData) const;
	bool FindAllSavedActors(TArray<FWarSaveGameData>& OutResult) const;
	bool RemoveSavedActorByID(const FGuid& InInstanceID) const;
	bool MarkAsDestroyed(const FGuid& InInstanceID) const;

	void InsertInventoryWithMax(const FInventoryItemInDB& InventoryItemInDB) const;
	void UpdateInventory(const FInventoryItemInDB& InventoryItemInDB) const;
	void MarkAsEquipped(const FGuid& InventoryID, const FGuid& PlayerID) const;
	void MarkAsUnEquipped(const FGuid& InventoryID, const FGuid& PlayerID) const;
	bool FindInventoryByState(const FGuid& PlayerID, const bool bEquippedState, TArray<FInventoryItemInDB>& OurResult) const;
	bool HasInventory(const FGuid& InventoryID, const FGuid& PlayerID) const;
	bool HasEquipped(const FGuid& InventoryID, const FGuid& PlayerID) const;
	bool FindInventoryByID(const FGuid& InventoryID, const FGuid& PlayerID, FInventoryItemInDB& OutResult) const;
	bool FindAllInventoriesByPlayerID(const FGuid& InPlayerID, TArray<FInventoryItemInDB>& OutResult) const;
	int32 FindAllInventoriesCount(const FGuid& InPlayerID) const;

	UWarPersistentSystem();

	UFUNCTION(BlueprintCallable)
	void SaveGame();
	UFUNCTION(BlueprintCallable)
	void LoadGame();

	//用于全部需要保存动态的物体生成持久的ID
	static void GeneratorPersistentID(AActor* Actor);
};
