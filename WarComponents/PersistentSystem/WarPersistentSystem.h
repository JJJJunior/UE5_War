#pragma once

#include "CoreMinimal.h"
#include "SQLitePreparedStatement.h"
#include "Templates/EnableIf.h"
#include "War/DataManager/DynamicData/InventoryData.h"
#include "War/GameInstance/WarGameInstanceSubSystem.h"
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

public:
	UWarPersistentSystem();
	UFUNCTION(BlueprintCallable)
	static void SaveGame(const UObject* WorldContextObject);
	UFUNCTION(BlueprintCallable)
	static void LoadGame(const UObject* WorldContextObject);

	FORCEINLINE static FGuid SetPersistentActorGuid() { return FGuid::NewGuid(); }

	static void DeleteAllInDB(const UObject* WorldContextObject);

	static void CheckInventoriesInDB(const UObject* WorldContextObject);
	static void InsertInventoryInDB(const UObject* WorldContextObject, const FInventoryItemInDB& InventoryItemInDB);
	static void DeleteInventoryByIDInDB(const UObject* WorldContextObject, const FGuid& InstanceID, const FGuid& InPlayerID);
	static bool UpdateInventoryDamageInDB(const UObject* WorldContextObject, const FGuid& PlayerID, const FGuid& InstanceID, float NewDamage);
	static bool UpdateInventoryInDB(const UObject* WorldContextObject, const FInventoryItemInDB& InventoryItemInDB);

	static TOptional<FInventoryItemInDB> GetInventoryByIDInDB(const UObject* WorldContextObject, const FGuid& InstanceID, const FGuid& InPlayerID);
	static TArray<FInventoryItemInDB> GetAllInventoryInDB(const UObject* WorldContextObject, const FGuid& InPlayerID);
	static TArray<FInventoryItemInDB> GetInventoryItemsByTypes(const UObject* WorldContextObject, const EWarInventoryType& InventoryType, const FGuid& InPlayerID);
};
