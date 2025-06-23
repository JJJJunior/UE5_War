#include "WarDataManager.h"
#include "GameInstance/WarGameInstanceSubSystem.h"
#include "War/DataManager/DynamicData/InventoryData.h"
#include "War/WarComponents/PersistentSystem/WarPersistentSystem.h"
#include "Tools/MyLog.h"
#include "WarComponents/InventorySystem/StaticData/WarInventoryDataTableRow.h"

bool UWarDataManager::CreateWeapon(const UObject* WorldContextObject, const FName& TableID, const FGuid& InPlayerID, FItemInBagData& ItemInBagData)
{
	if (!WorldContextObject) return false;
	const FWarInventoryRow* ItemRow = UWarGameInstanceSubSystem::FindInventoryRow(WorldContextObject, TableID);
	if (!ItemRow) return false;
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
		UWarGameInstanceSubSystem* Subsystem = WorldContextObject->GetWorld()->GetGameInstance()->GetSubsystem<UWarGameInstanceSubSystem>();
		if (!Subsystem) return false;
		UWarPersistentSystem* PersistentSystem = Subsystem->GetWarPersistentSystem();
		if (!PersistentSystem) return false;
		PersistentSystem->InsertInventory(NewWeapon);

		//构造背包数据
		ItemInBagData = FItemInBagData::CreateInBagData(Params.InstanceID, Params.TableRowID, Params.Count, Params.InventoryType);
		return true;
	}
	return false;
}


bool UWarDataManager::CreateArmor(const UObject* WorldContextObject, const FName& TableID, const FGuid& InPlayerID, FItemInBagData& ItemInBagData)
{
	if (!WorldContextObject) return false;
	const FWarInventoryRow* ItemRow = UWarGameInstanceSubSystem::FindInventoryRow(WorldContextObject, TableID);
	if (!ItemRow) return false;
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
		//持久化存储
		UWarGameInstanceSubSystem* Subsystem = WorldContextObject->GetWorld()->GetGameInstance()->GetSubsystem<UWarGameInstanceSubSystem>();
		if (!Subsystem) return false;
		UWarPersistentSystem* PersistentSystem = Subsystem->GetWarPersistentSystem();
		if (!PersistentSystem) return false;
		PersistentSystem->InsertInventory(NewArmor);
		//构造背包数据
		ItemInBagData = FItemInBagData::CreateInBagData(Params.InstanceID, Params.TableRowID, Params.Count, Params.InventoryType);
		return true;
	}
	return false;
}

bool UWarDataManager::CreateConsumable(const UObject* WorldContextObject, const FName& TableID, const FGuid& InPlayerID, FItemInBagData& ItemInBagData)
{
	if (!WorldContextObject) return false;
	const FWarInventoryRow* ItemRow = UWarGameInstanceSubSystem::FindInventoryRow(WorldContextObject, TableID);
	if (!ItemRow) return false;
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
		//持久化存储
		UWarGameInstanceSubSystem* Subsystem = WorldContextObject->GetWorld()->GetGameInstance()->GetSubsystem<UWarGameInstanceSubSystem>();
		if (!Subsystem) return false;
		UWarPersistentSystem* PersistentSystem = Subsystem->GetWarPersistentSystem();
		if (!PersistentSystem) return false;
		PersistentSystem->InsertInventory(NewConsumable);
		//构造背包数据
		ItemInBagData = FItemInBagData::CreateInBagData(Params.InstanceID, Params.TableRowID, Params.Count, Params.InventoryType);
		return true;
	}
	return false;
}

bool UWarDataManager::CreateQuestItem(const UObject* WorldContextObject, const FName& TableID, const FGuid& InPlayerID, FItemInBagData& ItemInBagData)
{
	if (!WorldContextObject) return false;
	const FWarInventoryRow* ItemRow = UWarGameInstanceSubSystem::FindInventoryRow(WorldContextObject, TableID);
	if (!ItemRow) return false;
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

		//持久化存储 
		UWarGameInstanceSubSystem* Subsystem = WorldContextObject->GetWorld()->GetGameInstance()->GetSubsystem<UWarGameInstanceSubSystem>();
		if (!Subsystem) return false;
		UWarPersistentSystem* PersistentSystem = Subsystem->GetWarPersistentSystem();
		if (!PersistentSystem) return false;
		PersistentSystem->InsertInventory(NewQuest);

		//构造背包数据
		ItemInBagData = FItemInBagData::CreateInBagData(Params.InstanceID, Params.TableRowID, Params.Count, Params.InventoryType);
		return true;
	}
	return false;
}

bool UWarDataManager::CreateSkill(const UObject* WorldContextObject, const FName& TableID, const FGuid& InPlayerID, FItemInBagData& ItemInBagData)
{
	if (!WorldContextObject) return false;
	const FWarInventoryRow* ItemRow = UWarGameInstanceSubSystem::FindInventoryRow(WorldContextObject, TableID);
	if (!ItemRow) return false;
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

		//持久化存储 
		UWarGameInstanceSubSystem* Subsystem = WorldContextObject->GetWorld()->GetGameInstance()->GetSubsystem<UWarGameInstanceSubSystem>();
		if (!Subsystem) return false;
		UWarPersistentSystem* PersistentSystem = Subsystem->GetWarPersistentSystem();
		if (!PersistentSystem) return false;
		PersistentSystem->InsertInventory(NewSkill);
		//构造背包数据
		ItemInBagData = FItemInBagData::CreateInBagData(Params.InstanceID, Params.TableRowID, Params.Count, Params.InventoryType);
		return true;
	}
	return false;
}
