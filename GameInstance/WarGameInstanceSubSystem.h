#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "War/WarComponents/InventorySystem/StaticData/WarInventoryDataTableRow.h"
#include "SQLiteDatabase.h"
#include "WarGameInstanceSubSystem.generated.h"

class UGameConfigData;
class UTexture2D;
class UWarDataManager;
class UWarPersistentSystem;

UCLASS()
class WAR_API UWarGameInstanceSubSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

protected:
	UPROPERTY()
	TObjectPtr<UWarPersistentSystem> WarPersistentSystem;
	UPROPERTY()
	TObjectPtr<UWarDataManager> WarDataManager;
	UPROPERTY()
	TObjectPtr<UGameConfigData> GameConfigData;
	UPROPERTY()
	TObjectPtr<UDataTable> WarInventoryDataTable;
	UPROPERTY()
	TObjectPtr<UDataTable> WarAbilityDataTable;


	TSoftObjectPtr<UMaterialInstance> GetOutLineMaterial() const;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	FSQLiteDatabase Database;
	FString DatabasePath = FPaths::ProjectSavedDir() / TEXT("Database/war.db");

public:
	UFUNCTION(BlueprintCallable)
	UWarPersistentSystem* GetWarPersistentSystem() const { return WarPersistentSystem; }

	UFUNCTION(BlueprintCallable)
	UWarDataManager* GetUWarDataManager() const { return WarDataManager; }

	FORCEINLINE TObjectPtr<UDataTable> GetCachedWarAbilityDataTable() const { return WarAbilityDataTable; }
	FORCEINLINE TObjectPtr<UGameConfigData> GetCachedGameConfigData() const { return GameConfigData; }
	//静态全局查找
	static const FWarInventoryRow* FindInventoryRow(const UObject* WorldContextObject, const FName& TableRowID);
	FSQLiteDatabase& GetDB() { return Database; }
};
