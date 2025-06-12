#include "WarGameInstanceSubSystem.h"

void UWarGameInstanceSubSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	WarSpawnDataTable = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/Game/ThirdPerson/InventorySystem/DT_InventoryData.DT_InventoryData")));
	check(!WarSpawnDataTable.IsNull());
	CachedWarSpawnDataTable = WarSpawnDataTable.LoadSynchronous();
	if (!CachedWarSpawnDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load DataTable from path: %s"), *WarSpawnDataTable.ToString());
	}

	WarAbilityDataTable = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/Game/ThirdPerson/DT_AbilityData.DT_AbilityData")));
	check(!WarAbilityDataTable.IsNull());
	CachedWarAbilityDataTable = WarAbilityDataTable.LoadSynchronous();
	if (!CachedWarAbilityDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load DataTable from path: %s"), *WarAbilityDataTable.ToString());
	}
}

TObjectPtr<UDataTable> UWarGameInstanceSubSystem::GetCachedWarInventoryDataTable() const
{
	check(CachedWarSpawnDataTable);
	return CachedWarSpawnDataTable;
}

TObjectPtr<UDataTable> UWarGameInstanceSubSystem::GetCachedWarAbilityDataTable() const
{
	check(CachedWarAbilityDataTable);
	return CachedWarAbilityDataTable;
}
