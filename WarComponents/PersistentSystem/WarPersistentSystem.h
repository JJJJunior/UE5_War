#pragma once

#include "CoreMinimal.h"
#include "DataManager/DynamicData/InventoryData.h"
#include "Templates/EnableIf.h"
#include "WarPersistentSystem.generated.h"


class UWarDataManager;


UCLASS()
class WAR_API UWarPersistentSystem : public UObject
{
	GENERATED_BODY()

public:
	FString SaveSlotName = TEXT("DefaultGame");
	int32 SaveSlotIndex = 0;

	FString SavedGameDataFieldName;
	FString SavedGameDataDBPath;
	TSharedPtr<FJsonObject> SavedGameDataJsonDB;
	FString InventoryFieldName;
	FString InventoryDBPath;
	TSharedPtr<FJsonObject> InventoryJsonDB;

	static bool SaveJsonToFile(const FString& JsonSavePath, const TSharedPtr<FJsonObject>& JsonObject);
	static bool InitJsonDB(const FString& JsonSavePath, TSharedPtr<FJsonObject>& OutJsonObject, const FString& DefaultFieldName, bool bInitAsArray);

	void InsertSavedActor(const FWarSaveGameData& SaveGameData) const;
	void UpdateSavedActor(const FWarSaveGameData& SaveGameData) const;
	bool FindAllSavedActors(TArray<FWarSaveGameData>& OutResult) const;
	bool RemoveSavedActorByID(const FGuid& InInstanceID) const;
	bool MarkAsDestroyed(const FGuid& InInstanceID) const;

	void InsertInventory(const FInventoryItemInDB& InventoryItemInDB) const;
	void UpdateInventory(const FInventoryItemInDB& InventoryItemInDB) const;
	bool MarkAsEquipped(const FGuid& InventoryID, const FGuid& PlayerID) const;
	bool MarkAsUnEquipped(const FGuid& InventoryID, const FGuid& PlayerID) const;
	bool FindEquippedInventory(const FGuid& InventoryID, const FGuid& PlayerID, TArray<FInventoryItemInDB>& OurResult) const;
	bool HasInventory(const FGuid& InventoryID, const FGuid& PlayerID) const;
	bool HasEquipped(const FGuid& InventoryID, const FGuid& PlayerID) const;
	bool FindInventoryByID(const FGuid& InventoryID, const FGuid& PlayerID, FInventoryItemInDB& OutResult) const;
	bool FindAllInventoriesByPlayerID(const FGuid& InPlayerID, TArray<FInventoryItemInDB>& OutResult) const;

	UWarPersistentSystem();

	UFUNCTION(BlueprintCallable)
	void SaveGame();
	UFUNCTION(BlueprintCallable)
	void LoadGame();

	//用于全部需要保存动态的物体生成持久的ID
	static void GeneratorPersistentID(AActor* Actor);
};
