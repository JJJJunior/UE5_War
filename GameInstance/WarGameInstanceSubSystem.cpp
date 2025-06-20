﻿#include "WarGameInstanceSubSystem.h"
#include "War/DataManager/ConfigData/GameConfigData.h"
#include "War/WarComponents/PersistentSystem/WarPersistentSystem.h"
#include "Tools/MyLog.h"


void UWarGameInstanceSubSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	// 创建软引用
	TSoftObjectPtr<UGameConfigData> SoftConfigData = TSoftObjectPtr<UGameConfigData>(FSoftObjectPath(TEXT("/Game/ThirdPerson/DA_GameConfig.DA_GameConfig")));
	// 同步加载资源
	GameConfigData = SoftConfigData.LoadSynchronous();
	checkf(GameConfigData, TEXT("GameConfigData is NULL"))

	WarInventoryDataTable = GameConfigData->WarInventoryDataTable.LoadSynchronous();
	WarAbilityDataTable = GameConfigData->WarAbilityDataTable.LoadSynchronous();
	if (!IsValid(WarInventoryDataTable))
	{
		print(TEXT("WarInventoryDataTable 空指针"))
		return;
	}
	if (!IsValid(WarAbilityDataTable))
	{
		print(TEXT("WarAbilityDataTable 空指针"))
		return;
	}

	WarPersistentSystem = NewObject<UWarPersistentSystem>(this);
	if (!IsValid(WarPersistentSystem))
	{
		UE_LOG(LogTemp, Error, TEXT("WarPersistentSystem is NULL"));
		return;
	}
}

TSoftObjectPtr<UMaterialInstance> UWarGameInstanceSubSystem::GetOutLineMaterial() const
{
	return GameConfigData->OutLineMaterial;
}

const FWarInventoryRow* UWarGameInstanceSubSystem::FindInventoryRow(const UObject* WorldContextObject, const FName& TableRowID)
{
	if (!WorldContextObject)
	{
		print(TEXT("WorldContextObject 不存在"))
		return nullptr;
	}
	UWarGameInstanceSubSystem* Subsystem = WorldContextObject->GetWorld()->GetGameInstance()->GetSubsystem<UWarGameInstanceSubSystem>();
	if (!Subsystem || !Subsystem->WarInventoryDataTable)
	{
		print(TEXT("Subsystem or WarInventoryDataTable 不存在"))
		return nullptr;
	}
	FWarInventoryRow* FindItemRow = Subsystem->WarInventoryDataTable->FindRow<FWarInventoryRow>(TableRowID, "FindInventoryRow");
	if (!FindItemRow)
	{
		print(TEXT("FindItemRow 不存在"))
		return nullptr;
	}
	return FindItemRow;
}
