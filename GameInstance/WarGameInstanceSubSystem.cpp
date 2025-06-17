#include "WarGameInstanceSubSystem.h"
#include "DataAssets/GameConfigData.h"

void UWarGameInstanceSubSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	// 创建软引用
	TSoftObjectPtr<UGameConfigData> SoftConfigData = TSoftObjectPtr<UGameConfigData>(FSoftObjectPath(TEXT("/Game/ThirdPerson/DA_GameConfig.DA_GameConfig")));
	// 同步加载资源
	GameConfigData = SoftConfigData.LoadSynchronous();
	checkf(GameConfigData, TEXT("GameConfigData is NULL"))
}

TObjectPtr<UDataTable> UWarGameInstanceSubSystem::GetCachedWarInventoryDataTable() const
{
	// 同步加载资源
	return GameConfigData->WarInventoryDataTable.LoadSynchronous();
}

TObjectPtr<UDataTable> UWarGameInstanceSubSystem::GetCachedWarAbilityDataTable() const
{
	// 同步加载资源
	return GameConfigData->WarAbilityDataTable.LoadSynchronous();
}

TSoftObjectPtr<UMaterialInstance> UWarGameInstanceSubSystem::GetOutLineMaterial() const
{
	return GameConfigData->OutLineMaterial;
}
