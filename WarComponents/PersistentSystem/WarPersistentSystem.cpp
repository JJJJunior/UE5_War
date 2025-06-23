#include "WarPersistentSystem.h"
#include "EngineUtils.h"
#include "GameInstance/WarGameInstanceSubSystem.h"
#include "SaveGame/WarSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Tools/MyLog.h"
#include "SaveGame/Interface/WarSaveGameInterface.h"


UWarPersistentSystem::UWarPersistentSystem()
{
}


void UWarPersistentSystem::SaveGame(const UObject* WorldContextObject)
{
	if (!WorldContextObject) return;
	UWorld* World = WorldContextObject->GetWorld();
	if (!World) return;
	UWarGameInstanceSubSystem* Subsystem = World->GetGameInstance()->GetSubsystem<UWarGameInstanceSubSystem>();
	if (!Subsystem) return;
	UWarPersistentSystem* PersistentSystem = Subsystem->GetWarPersistentSystem();
	if (!PersistentSystem) return;

	UWarSaveGame* SaveGameInstance = Cast<UWarSaveGame>(UGameplayStatics::CreateSaveGameObject(UWarSaveGame::StaticClass()));

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (IWarSaveGameInterface* SaveInterface = Cast<IWarSaveGameInterface>(Actor))
		{
			//存档数据构造
			FWarSaveGameData ActorData;
			//通过接口获取存储在Actor对象身上的唯一id
			ActorData.ActorGuid = SaveInterface->GetPersistentActorID();
			ActorData.ActorTransform = Actor->GetActorTransform();
			FMemoryWriter MemoryWriter(ActorData.ActorData, true);
			SaveInterface->SaveActorData(MemoryWriter);

			//存实例
			SaveGameInstance->GameSavedActors.Add(ActorData);
		}
	}
	print(TEXT("游戏已保存------------- "));
	UGameplayStatics::SaveGameToSlot(SaveGameInstance, TEXT("MySaveSlot"), 0);
}

void UWarPersistentSystem::LoadGame(const UObject* WorldContextObject)
{
	if (!WorldContextObject) return;

	UWarSaveGame* LoadGameInstance = Cast<UWarSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("MySaveSlot"), 0));
	if (!LoadGameInstance) return;

	UWorld* World = WorldContextObject->GetWorld();
	if (!World) return;

	UWarGameInstanceSubSystem* Subsystem = World->GetGameInstance()->GetSubsystem<UWarGameInstanceSubSystem>();
	if (!Subsystem) return;
	UWarPersistentSystem* PersistentSystem = Subsystem->GetWarPersistentSystem();
	if (!PersistentSystem) return;

	for (const FWarSaveGameData& ActorData : LoadGameInstance->GameSavedActors)
	{
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;

			if (IWarSaveGameInterface* SaveInterface = Cast<IWarSaveGameInterface>(Actor))
			{
				if (SaveInterface->GetPersistentActorID() == ActorData.ActorGuid)
				{
					// 还原 Transform
					Actor->SetActorTransform(ActorData.ActorTransform);
					// 还原状态数据
					FMemoryReader MemoryReader(ActorData.ActorData, true);
					SaveInterface->LoadActorData(MemoryReader);
					break;
				}
			}
		}
	}
	// print(TEXT("游戏读取完毕-------------"));
}


void UWarPersistentSystem::CheckInventoriesInDB(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		print(TEXT("WorldContextObject 不存在"))
		return;
	}
	UWarGameInstanceSubSystem* Subsystem = WorldContextObject->GetWorld()->GetGameInstance()->GetSubsystem<UWarGameInstanceSubSystem>();
	if (!IsValid(Subsystem)) return;
	UWarPersistentSystem* PersistentSystem = Subsystem->GetWarPersistentSystem();
	if (!IsValid(PersistentSystem)) return;
}


void UWarPersistentSystem::InsertInventoryInDB(const UObject* WorldContextObject, const FInventoryItemInDB& InventoryItemInDB)
{
	if (!WorldContextObject) return;
	UWorld* World = WorldContextObject->GetWorld();
	if (!World) return;
	UWarGameInstanceSubSystem* Subsystem = World->GetGameInstance()->GetSubsystem<UWarGameInstanceSubSystem>();

	if (!InventoryItemInDB.InstanceID.IsValid())
	{
		print(TEXT("InstanceID %s  无效"), *InventoryItemInDB.InstanceID.ToString());
		return;
	}
	if (!InventoryItemInDB.PlayerID.IsValid())
	{
		print(TEXT("PlayerID %s  无效"), *InventoryItemInDB.PlayerID.ToString());
		return;
	}

	if (!InventoryItemInDB.TableRowID.IsValid())
	{
		print(TEXT("TableRowID %s  无效"), *InventoryItemInDB.TableRowID.ToString());
		return;
	}

	FString SQL = TEXT(
		"INSERT INTO InventoryItems "
		"(instanceid,playerid,tablerowid,count,inventorytype,damage,durability,defense,questid,level,consumable_amount,cooldown) "
		"VALUES (?,?,?,?,?,?,?,?,?,?,?,?);");

	FSQLitePreparedStatement Statement(Subsystem->GetDB(), *SQL);
	Statement.SetBindingValueByIndex(1, InventoryItemInDB.InstanceID.ToString());
	Statement.SetBindingValueByIndex(2, InventoryItemInDB.PlayerID.ToString());
	Statement.SetBindingValueByIndex(3, InventoryItemInDB.TableRowID.ToString());
	Statement.SetBindingValueByIndex(4, InventoryItemInDB.Count);
	Statement.SetBindingValueByIndex(5, InventoryItemInDB.InventoryType);
	Statement.SetBindingValueByIndex(6, InventoryItemInDB.Damage);
	Statement.SetBindingValueByIndex(7, InventoryItemInDB.Durability);
	Statement.SetBindingValueByIndex(8, InventoryItemInDB.Defense);
	Statement.SetBindingValueByIndex(9, InventoryItemInDB.QuestID);
	Statement.SetBindingValueByIndex(10, InventoryItemInDB.Level);
	Statement.SetBindingValueByIndex(11, InventoryItemInDB.Amount);
	Statement.SetBindingValueByIndex(12, InventoryItemInDB.Cooldown);

	if (Statement.Execute())
	{
		print(TEXT("Insert successful."));
	}
	else
	{
		print(TEXT("Insert failed."));
		print(TEXT("%s"), *SQL)
	}
	Statement.Reset();
}

bool UWarPersistentSystem::UpdateInventoryInDB(const UObject* WorldContextObject, const FInventoryItemInDB& InventoryItemInDB)
{
	if (!WorldContextObject) return false;

	UWorld* World = WorldContextObject->GetWorld();
	if (!World) return false;

	UWarGameInstanceSubSystem* Subsystem = World->GetGameInstance()->GetSubsystem<UWarGameInstanceSubSystem>();
	if (!Subsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("UWarGameInstanceSubSystem is nullptr"));
		return false;
	}

	if (InventoryItemInDB.InstanceID.IsValid())
	{
		print(TEXT("InstanceID %s  无效"), *InventoryItemInDB.InstanceID.ToString());
		return false;
	}
	if (InventoryItemInDB.PlayerID.IsValid())
	{
		print(TEXT("PlayerID %s  无效"), *InventoryItemInDB.PlayerID.ToString());
		return false;
	}

	if (InventoryItemInDB.TableRowID.IsValid())
	{
		print(TEXT("TableRowID %s  无效"), *InventoryItemInDB.TableRowID.ToString());
		return false;
	}

	// 更新SQL
	FString SQL = TEXT(
		"UPDATE InventoryItems SET "
		"tablerowid = ?, "
		"count = ?, "
		"inventorytype = ?, "
		"damage = ?, "
		"durability = ?, "
		"defense = ?, "
		"questid = ?, "
		"level = ?, "
		"consumable_amount = ?, "
		"cooldown = ? "
		"WHERE playerid = ? AND instanceid = ?;"
	);

	// 准备预处理语句
	FSQLitePreparedStatement Statement(Subsystem->GetDB(), *SQL);

	// 参数绑定（注意索引从 1 开始）
	Statement.SetBindingValueByIndex(1, InventoryItemInDB.TableRowID.ToString());
	Statement.SetBindingValueByIndex(2, InventoryItemInDB.Count);
	Statement.SetBindingValueByIndex(3, static_cast<int32>(InventoryItemInDB.InventoryType));
	Statement.SetBindingValueByIndex(4, InventoryItemInDB.Damage);
	Statement.SetBindingValueByIndex(5, InventoryItemInDB.Durability);
	Statement.SetBindingValueByIndex(6, InventoryItemInDB.Defense);
	Statement.SetBindingValueByIndex(7, InventoryItemInDB.QuestID.ToString());
	Statement.SetBindingValueByIndex(8, InventoryItemInDB.Level);
	Statement.SetBindingValueByIndex(9, InventoryItemInDB.Amount);
	Statement.SetBindingValueByIndex(10, InventoryItemInDB.Cooldown);

	// WHERE 条件参数
	Statement.SetBindingValueByIndex(11, InventoryItemInDB.PlayerID.ToString());
	Statement.SetBindingValueByIndex(12, InventoryItemInDB.InstanceID.ToString());

	// 执行更新
	if (Statement.Step() == ESQLitePreparedStatementStepResult::Done)
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully updated inventory item %s"), *InventoryItemInDB.InstanceID.ToString());
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to update inventory item %s"), *InventoryItemInDB.InstanceID.ToString());
		return false;
	}
}


void UWarPersistentSystem::DeleteAllInDB(const UObject* WorldContextObject)
{
	if (!WorldContextObject) return;

	UWorld* World = WorldContextObject->GetWorld();
	if (!World) return;

	UWarGameInstanceSubSystem* Subsystem = World->GetGameInstance()->GetSubsystem<UWarGameInstanceSubSystem>();
	if (!Subsystem)
	{
		print_err(TEXT("UWarGameInstanceSubSystem is nullptr"));
		return;
	}

	const FString SQL = TEXT("DELETE FROM inventoryitems");

	FSQLitePreparedStatement Statement(Subsystem->GetDB(), *SQL);
	// 执行更新
	if (Statement.Step() == ESQLitePreparedStatementStepResult::Done)
	{
		print(TEXT("Successfully delete all"));
		return;
	}
	print_err(TEXT("Failed to delete all"));
}


void UWarPersistentSystem::DeleteInventoryByIDInDB(const UObject* WorldContextObject, const FGuid& InstanceID, const FGuid& InPlayerID)
{
	if (!WorldContextObject) return;

	UWorld* World = WorldContextObject->GetWorld();
	if (!World) return;

	UWarGameInstanceSubSystem* Subsystem = World->GetGameInstance()->GetSubsystem<UWarGameInstanceSubSystem>();
	if (!Subsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("UWarGameInstanceSubSystem is nullptr"));
		return;
	}

	if (!InstanceID.IsValid())
	{
		print(TEXT("InstanceID %s  无效"), *InstanceID.ToString());
		return;
	}
	if (!InPlayerID.IsValid())
	{
		print(TEXT("InPlayerID %s  无效"), *InPlayerID.ToString());
		return;
	}

	const FString SQL = TEXT("DELETE FROM inventoryitems WHERE instanceid = ? AND playerid = ?");
	FSQLitePreparedStatement Statement(Subsystem->GetDB(), *SQL);
	Statement.SetBindingValueByIndex(1, InstanceID.ToString());
	Statement.SetBindingValueByIndex(2, InPlayerID.ToString());

	// 执行更新
	if (Statement.Step() == ESQLitePreparedStatementStepResult::Done)
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully delete inventory item %s"), *InstanceID.ToString());
		return;
	}
	UE_LOG(LogTemp, Error, TEXT("Failed to delete inventory item %s"), *InstanceID.ToString());
}

bool UWarPersistentSystem::UpdateInventoryDamageInDB(const UObject* WorldContextObject, const FGuid& InPlayerID, const FGuid& InstanceID, float NewDamage)
{
	if (!WorldContextObject) return false;

	UWorld* World = WorldContextObject->GetWorld();
	if (!World) return false;

	UWarGameInstanceSubSystem* Subsystem = World->GetGameInstance()->GetSubsystem<UWarGameInstanceSubSystem>();
	if (!Subsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("UWarGameInstanceSubSystem is nullptr"));
		return false;
	}

	if (!InstanceID.IsValid())
	{
		print(TEXT("InstanceID %s  无效"), *InstanceID.ToString());
		return false;
	}
	if (!InPlayerID.IsValid())
	{
		print(TEXT("InPlayerID %s  无效"), *InPlayerID.ToString());
		return false;
	}

	// 单字段更新 SQL
	const FString SQL = TEXT("UPDATE inventoryitems SET damage = ? WHERE playerid = ? AND instanceid = ?");

	FSQLitePreparedStatement Statement(Subsystem->GetDB(), *SQL);
	// 绑定参数 (注意索引从 1 开始)
	Statement.SetBindingValueByIndex(1, NewDamage); // damage
	Statement.SetBindingValueByIndex(2, InPlayerID.ToString()); // playerid
	Statement.SetBindingValueByIndex(3, InstanceID.ToString()); // instanceid

	// 执行更新
	if (Statement.Step() == ESQLitePreparedStatementStepResult::Done)
	{
		print(TEXT("Successfully updated damage for inventory item %s"), *InstanceID.ToString())
		return true;
	}
	print_err(TEXT("Failed to update inventory item %s"), *InstanceID.ToString());
	return false;
}


TOptional<FInventoryItemInDB> UWarPersistentSystem::GetInventoryByIDInDB(const UObject* WorldContextObject, const FGuid& InstanceID, const FGuid& InPlayerID)
{
	if (!WorldContextObject) return TOptional<FInventoryItemInDB>();
	UWorld* World = WorldContextObject->GetWorld();
	if (!World) return TOptional<FInventoryItemInDB>();

	UWarGameInstanceSubSystem* Subsystem = World->GetGameInstance()->GetSubsystem<UWarGameInstanceSubSystem>();

	if (!InstanceID.IsValid())
	{
		print(TEXT("InstanceID %s  无效"), *InstanceID.ToString());
		return TOptional<FInventoryItemInDB>();
	}
	if (!InPlayerID.IsValid())
	{
		print(TEXT("InPlayerID %s  无效"), *InPlayerID.ToString());
		return TOptional<FInventoryItemInDB>();
	}

	const FString& SQL = TEXT("SELECT * FROM inventoryitems WHERE playerid = ? AND instanceid = ?");

	FSQLitePreparedStatement Statement(Subsystem->GetDB(), *SQL);
	Statement.SetBindingValueByIndex(1, InPlayerID.ToString());
	Statement.SetBindingValueByIndex(1, InstanceID.ToString());
	if (Statement.Step() == ESQLitePreparedStatementStepResult::Row)
	{
		FInventoryItemInDB ResultItem;

		FString InstanceIDString, PlayerIDString, TableRowIDString;
		Statement.GetColumnValueByName(TEXT("instanceid"), InstanceIDString);
		Statement.GetColumnValueByName(TEXT("playerid"), PlayerIDString);
		Statement.GetColumnValueByName(TEXT("tablerowid"), TableRowIDString);

		FGuid::Parse(InstanceIDString, ResultItem.InstanceID);
		FGuid::Parse(PlayerIDString, ResultItem.PlayerID);
		ResultItem.TableRowID = FName(*TableRowIDString);

		Statement.GetColumnValueByName(TEXT("inventorytype"), ResultItem.InventoryType);
		Statement.GetColumnValueByName(TEXT("count"), ResultItem.Count);
		Statement.GetColumnValueByName(TEXT("damage"), ResultItem.Damage);
		Statement.GetColumnValueByName(TEXT("durability"), ResultItem.Durability);
		Statement.GetColumnValueByName(TEXT("defense"), ResultItem.Defense);
		Statement.GetColumnValueByName(TEXT("questid"), ResultItem.QuestID);
		Statement.GetColumnValueByName(TEXT("level"), ResultItem.Level);
		Statement.GetColumnValueByName(TEXT("consumable_amount"), ResultItem.Amount);
		Statement.GetColumnValueByName(TEXT("cooldown"), ResultItem.Cooldown);

		return ResultItem;
	}
	return TOptional<FInventoryItemInDB>();
}

TArray<FInventoryItemInDB> UWarPersistentSystem::GetAllInventoryInDB(const UObject* WorldContextObject, const FGuid& InPlayerID)
{
	if (!WorldContextObject) return TArray<FInventoryItemInDB>();
	UWorld* World = WorldContextObject->GetWorld();
	if (!World) return TArray<FInventoryItemInDB>();
	UWarGameInstanceSubSystem* Subsystem = World->GetGameInstance()->GetSubsystem<UWarGameInstanceSubSystem>();

	if (!InPlayerID.IsValid())
	{
		print(TEXT("InPlayerID %s  无效"), *InPlayerID.ToString());
		return TArray<FInventoryItemInDB>();
	}

	TArray<FInventoryItemInDB> Results;
	const FString& SQL = TEXT("SELECT * FROM inventoryitems WHERE playerid = ?");

	FSQLitePreparedStatement Statement(Subsystem->GetDB(), *SQL);
	Statement.SetBindingValueByIndex(1, InPlayerID.ToString());
	while (true)
	{
		const ESQLitePreparedStatementStepResult& StepResult = Statement.Step();
		if (StepResult == ESQLitePreparedStatementStepResult::Row)
		{
			FInventoryItemInDB Item;

			//基础字段
			FString InstanceIDString, PlayerIDString, TableRowIDString;

			Statement.GetColumnValueByName(TEXT("instanceid"), InstanceIDString);
			Statement.GetColumnValueByName(TEXT("playerid"), PlayerIDString);
			Statement.GetColumnValueByName(TEXT("tablerowid"), TableRowIDString);

			FGuid::Parse(InstanceIDString, Item.InstanceID);
			FGuid::Parse(PlayerIDString, Item.PlayerID);
			Item.TableRowID = FName(*TableRowIDString);

			Statement.GetColumnValueByName(TEXT("inventorytype"), Item.InventoryType);
			Statement.GetColumnValueByName(TEXT("count"), Item.Count);
			Statement.GetColumnValueByName(TEXT("damage"), Item.Damage);
			Statement.GetColumnValueByName(TEXT("durability"), Item.Durability);
			Statement.GetColumnValueByName(TEXT("defense"), Item.Defense);
			Statement.GetColumnValueByName(TEXT("questid"), Item.QuestID);
			Statement.GetColumnValueByName(TEXT("level"), Item.Level);
			Statement.GetColumnValueByName(TEXT("consumable_amount"), Item.Amount);
			Statement.GetColumnValueByName(TEXT("cooldown"), Item.Cooldown);
			Results.Add(Item);
		}
		else if (StepResult == ESQLitePreparedStatementStepResult::Done)
		{
			// 查询结束，退出循环
			break;
		}
		else
		{
			// 查询出错，建议也 break 防止死锁
			break;
		}
	}
	Statement.Reset();
	return Results;
}


TArray<FInventoryItemInDB> UWarPersistentSystem::GetInventoryItemsByTypes(const UObject* WorldContextObject, const EWarInventoryType& InventoryType, const FGuid& InPlayerID)
{
	if (!WorldContextObject) return TArray<FInventoryItemInDB>();
	UWorld* World = WorldContextObject->GetWorld();
	if (!World) return TArray<FInventoryItemInDB>();
	UWarGameInstanceSubSystem* Subsystem = World->GetGameInstance()->GetSubsystem<UWarGameInstanceSubSystem>();

	if (!InPlayerID.IsValid())
	{
		print(TEXT("InPlayerID %s  无效"), *InPlayerID.ToString());
		return TArray<FInventoryItemInDB>();
	}

	TArray<FInventoryItemInDB> Results;
	const FString& SQL = TEXT("SELECT * FROM inventoryitems WHERE playerid = ? AND inventorytype = ?");

	FSQLitePreparedStatement Statement(Subsystem->GetDB(), *SQL);
	Statement.SetBindingValueByIndex(1, InPlayerID.ToString());
	Statement.SetBindingValueByIndex(2, static_cast<int32>(InventoryType));
	while (true)
	{
		const ESQLitePreparedStatementStepResult& StepResult = Statement.Step();
		if (StepResult == ESQLitePreparedStatementStepResult::Row)
		{
			FInventoryItemInDB Item;

			//基础字段
			FString InstanceIDString, PlayerIDString, TableRowIDString;

			Statement.GetColumnValueByName(TEXT("instanceid"), InstanceIDString);
			Statement.GetColumnValueByName(TEXT("playerid"), PlayerIDString);
			Statement.GetColumnValueByName(TEXT("tablerowid"), TableRowIDString);

			FGuid::Parse(InstanceIDString, Item.InstanceID);
			FGuid::Parse(PlayerIDString, Item.PlayerID);
			Item.TableRowID = FName(*TableRowIDString);

			Statement.GetColumnValueByName(TEXT("inventorytype"), Item.InventoryType);
			Statement.GetColumnValueByName(TEXT("count"), Item.Count);
			Statement.GetColumnValueByName(TEXT("damage"), Item.Damage);
			Statement.GetColumnValueByName(TEXT("durability"), Item.Durability);
			Statement.GetColumnValueByName(TEXT("defense"), Item.Defense);
			Statement.GetColumnValueByName(TEXT("questid"), Item.QuestID);
			Statement.GetColumnValueByName(TEXT("level"), Item.Level);
			Statement.GetColumnValueByName(TEXT("consumable_amount"), Item.Amount);
			Statement.GetColumnValueByName(TEXT("cooldown"), Item.Cooldown);
			Results.Add(Item);
		}
		else if (StepResult == ESQLitePreparedStatementStepResult::Done)
		{
			// 查询结束，退出循环
			break;
		}
		else
		{
			// 查询出错，建议也 break 防止死锁
			break;
		}
	}
	Statement.Reset();
	return Results;
}

void UWarPersistentSystem::CheckInventoriesInDB(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		print(TEXT("WorldContextObject 不存在"))
		return;
	}
	UWarGameInstanceSubSystem* Subsystem = WorldContextObject->GetWorld()->GetGameInstance()->GetSubsystem<UWarGameInstanceSubSystem>();
	if (!IsValid(Subsystem)) return;
	UWarPersistentSystem* PersistentSystem = Subsystem->GetWarPersistentSystem();
	if (!IsValid(PersistentSystem)) return;

	print(TEXT("GameSavedWeapons %d"), PersistentSystem->GameSavedWeapons.Num())
	print(TEXT("GameSavedArmors %d"), PersistentSystem->GameSavedArmors.Num())
	print(TEXT("GameSaveInventories %d"), PersistentSystem->GameSavedInventories.Num())
	print(TEXT("GameSavedQuestItems %d"), PersistentSystem->GameSavedQuestItems.Num())
	print(TEXT("GameSavedConsumables %d"), PersistentSystem->GameSavedConsumables.Num())
	print(TEXT("GameSavedSkills %d"), PersistentSystem->GameSavedSkills.Num())
}


bool UWarPersistentSystem::AddInventoryToDB(const UObject* WorldContextObject, const FInventoryItemData& ItemData)
{
	if (!WorldContextObject) { return false; }
	UWarGameInstanceSubSystem* Subsystem = WorldContextObject->GetWorld()->GetGameInstance()->GetSubsystem<UWarGameInstanceSubSystem>();
	if (!Subsystem) return false;
	UWarPersistentSystem* PersistentSystem = Subsystem->GetWarPersistentSystem();
	if (!PersistentSystem) return false;

	const FGuid& ID = ItemData.InstanceID;

	// 检查ID是否已存在
	if (PersistentSystem->IsInstanceIDExist(ID))
	{
		UE_LOG(LogTemp, Warning, TEXT("物品ID已存在: %s"), *ID.ToString());
		return false;
	}

	// 根据类型存储
	switch (ItemData.InventoryType)
	{
	case EWarInventoryType::Weapon:
		if (ItemData.InventoryType == EWarInventoryType::Weapon)
		{
			const FWeaponData& Weapon = static_cast<const FWeaponData&>(ItemData);
			PersistentSystem->GameSavedWeapons.Add(ID, Weapon);
			return true;
		}
		break;
	case EWarInventoryType::Armor:
		if (ItemData.InventoryType == EWarInventoryType::Armor)
		{
			const FArmorData& Armor = static_cast<const FArmorData&>(ItemData);
			PersistentSystem->GameSavedArmors.Add(ID, Armor);
			return true;
		}
		break;

	case EWarInventoryType::Skill:
		if (ItemData.InventoryType == EWarInventoryType::Skill)
		{
			const FSkillData& Skill = static_cast<const FSkillData&>(ItemData);
			PersistentSystem->GameSavedSkills.Add(ID, Skill);
			return true;
		}
		break;

	case EWarInventoryType::Consumable:
		if (ItemData.InventoryType == EWarInventoryType::Consumable)
		{
			const FConsumableData& Consumable = static_cast<const FConsumableData&>(ItemData);
			PersistentSystem->GameSavedConsumables.Add(ID, Consumable);
			return true;
		}
		break;

	case EWarInventoryType::QuestItem:
		if (ItemData.InventoryType == EWarInventoryType::QuestItem)
		{
			const FQuestItemData& QuestItem = static_cast<const FQuestItemData&>(ItemData);
			PersistentSystem->GameSavedQuestItems.Add(ID, QuestItem);
			return true;
		}
		break;
	case EWarInventoryType::Material:
		break;
	default:
		break;
	}
	return false;
}

bool UWarPersistentSystem::IsInstanceIDExist(const FGuid& ID) const
{
	return GameSavedInventories.Contains(ID) ||
		GameSavedWeapons.Contains(ID) ||
		GameSavedArmors.Contains(ID) ||
		GameSavedQuestItems.Contains(ID) ||
		GameSavedSkills.Contains(ID) ||
		GameSavedConsumables.Contains(ID);
}
