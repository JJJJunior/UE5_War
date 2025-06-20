#include "War/DataManager/DynamicData/InventoryData.h"
#include "War/WarComponents/PersistentSystem/WarPersistentSystem.h"
#include "WarDataManager.h"
#include "Tools/MyLog.h"

TOptional<FItemInBagData> UWarDataManager::CreateWeapon(const UObject* WorldContextObject, const FName& TableID, const FGuid& InPlayerID)
{
	if (!WorldContextObject) return TOptional<FItemInBagData>();
	const FWarInventoryRow* ItemRow = UWarGameInstanceSubSystem::FindInventoryRow(WorldContextObject, TableID);
	if (!ItemRow) return TOptional<FItemInBagData>();
	if (ItemRow->InventoryType == EWarInventoryType::Weapon)
	{
		FInventoryItemInDBParams Params;
		Params.TableRowID = TableID;
		Params.InventoryType = ItemRow->InventoryType;
		Params.InstanceID = FGuid::NewGuid();
		Params.PlayerID = InPlayerID;
		Params.Count = 1;
		Params.Damage = 112.5f;
		Params.Durability = 100;

		print(TEXT("CreateWeapon.InPlayerID  %s"), *InPlayerID.ToString())
		print(TEXT("CreateWeapon.Params.PlayerID  %s"), *Params.PlayerID.ToString())

		const FInventoryItemInDB& NewWeapon = FInventoryItemInDB::Init(Params);

		//持久化存储 sqlite
		UWarPersistentSystem::InsertInventoryInDB(WorldContextObject, NewWeapon);
		//构造背包数据
		const FItemInBagData& BagData = FItemInBagData::CreateInBagData(Params.InstanceID, Params.TableRowID, Params.Count, Params.InventoryType);
		return BagData;
	}
	return TOptional<FItemInBagData>();
}


TOptional<FItemInBagData> UWarDataManager::CreateArmor(const UObject* WorldContextObject, const FName& TableID, const FGuid& InPlayerID)
{
	if (!WorldContextObject) return TOptional<FItemInBagData>();
	const FWarInventoryRow* ItemRow = UWarGameInstanceSubSystem::FindInventoryRow(WorldContextObject, TableID);
	if (!ItemRow) return TOptional<FItemInBagData>();
	if (ItemRow->InventoryType == EWarInventoryType::Armor)
	{
		FInventoryItemInDBParams Params;
		Params.TableRowID = TableID;
		Params.InventoryType = ItemRow->InventoryType;
		Params.InstanceID = FGuid::NewGuid();
		Params.PlayerID = InPlayerID;
		Params.Count = 1;
		Params.Defense = 100.0f;
		Params.Durability = 100;

		const FInventoryItemInDB& NewArmor = FInventoryItemInDB::Init(Params);

		//持久化存储 sqlite
		UWarPersistentSystem::InsertInventoryInDB(WorldContextObject, NewArmor);
		//构造背包数据
		const FItemInBagData& BagData = FItemInBagData::CreateInBagData(Params.InstanceID, Params.TableRowID, Params.Count, Params.InventoryType);
		return BagData;
	}
	return TOptional<FItemInBagData>();
}

TOptional<FItemInBagData> UWarDataManager::CreateConsumable(const UObject* WorldContextObject, const FName& TableID, const FGuid& InPlayerID)
{
	if (!WorldContextObject) return TOptional<FItemInBagData>();
	const FWarInventoryRow* ItemRow = UWarGameInstanceSubSystem::FindInventoryRow(WorldContextObject, TableID);
	if (!ItemRow) return TOptional<FItemInBagData>();
	if (ItemRow->InventoryType == EWarInventoryType::Consumable)
	{
		FInventoryItemInDBParams Params;
		Params.TableRowID = TableID;
		Params.InventoryType = ItemRow->InventoryType;
		Params.InstanceID = FGuid::NewGuid();
		Params.PlayerID = InPlayerID;
		Params.Count = 1;
		Params.Amount = 0.f;

		const FInventoryItemInDB& NewConsumable = FInventoryItemInDB::Init(Params);
		//持久化存储 sqlite
		UWarPersistentSystem::InsertInventoryInDB(WorldContextObject, NewConsumable);
		//构造背包数据
		const FItemInBagData& BagData = FItemInBagData::CreateInBagData(Params.InstanceID, Params.TableRowID, Params.Count, Params.InventoryType);
		return BagData;
	}
	return TOptional<FItemInBagData>();
}

TOptional<FItemInBagData> UWarDataManager::CreateQuestItem(const UObject* WorldContextObject, const FName& TableID, const FGuid& InPlayerID)
{
	if (!WorldContextObject) return TOptional<FItemInBagData>();
	const FWarInventoryRow* ItemRow = UWarGameInstanceSubSystem::FindInventoryRow(WorldContextObject, TableID);
	if (!ItemRow) return TOptional<FItemInBagData>();
	if (ItemRow->InventoryType == EWarInventoryType::QuestItem)
	{
		FInventoryItemInDBParams Params;
		Params.TableRowID = TableID;
		Params.InventoryType = ItemRow->InventoryType;
		Params.InstanceID = FGuid::NewGuid();
		Params.PlayerID = InPlayerID;
		Params.Count = 1;
		Params.QuestID = FGuid::NewGuid();

		const FInventoryItemInDB& NewQuest = FInventoryItemInDB::Init(Params);

		//持久化存储 sqlite
		UWarPersistentSystem::InsertInventoryInDB(WorldContextObject, NewQuest);
		//构造背包数据
		const FItemInBagData& BagData = FItemInBagData::CreateInBagData(Params.InstanceID, Params.TableRowID, Params.Count, Params.InventoryType);
		return BagData;
	}
	return TOptional<FItemInBagData>();
}

TOptional<FItemInBagData> UWarDataManager::CreateSkill(const UObject* WorldContextObject, const FName& TableID, const FGuid& InPlayerID)
{
	if (!WorldContextObject) return TOptional<FItemInBagData>();
	const FWarInventoryRow* ItemRow = UWarGameInstanceSubSystem::FindInventoryRow(WorldContextObject, TableID);
	if (!ItemRow) return TOptional<FItemInBagData>();
	if (ItemRow->InventoryType == EWarInventoryType::Skill)
	{
		FInventoryItemInDBParams Params;
		Params.TableRowID = TableID;
		Params.InventoryType = ItemRow->InventoryType;
		Params.InstanceID = FGuid::NewGuid();
		Params.PlayerID = InPlayerID;
		Params.Count = 1;
		Params.Cooldown = 0.f;

		const FInventoryItemInDB& NewSkill = FInventoryItemInDB::Init(Params);

		//持久化存储 sqlite
		UWarPersistentSystem::InsertInventoryInDB(WorldContextObject, NewSkill);
		//构造背包数据
		const FItemInBagData& BagData = FItemInBagData::CreateInBagData(Params.InstanceID, Params.TableRowID, Params.Count, Params.InventoryType);
		return BagData;
	}
	return TOptional<FItemInBagData>();
}
