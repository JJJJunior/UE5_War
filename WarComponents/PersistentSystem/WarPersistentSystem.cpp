#include "WarPersistentSystem.h"
#include <devicetopology.h>
#include "EngineUtils.h"
#include "Characters/Hero/WarHeroCharacter.h"
#include "DataManager/ConfigData/GameConfigData.h"
#include "DataManager/DynamicData/InventoryData.h"
#include "GameInstance/WarGameInstanceSubSystem.h"
#include "War/WarComponents/PersistentSystem/Interface/WarSaveGameInterface.h"
#include "Tools/MyLog.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "War/WarComponents/PersistentSystem/WarJsonHelper.h"
#include "WorldActors/Inventory/InventoryBase.h"

UWarPersistentSystem::UWarPersistentSystem()
{
	InitSaveActors();
	InitInventories();
}


void UWarPersistentSystem::InitSaveActors()
{
	SavedGameDataFieldName = "SavedActors";
	SavedGameDataDBPath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Database/"), TEXT("SavedGameDataDB.json"));
	if (InitJsonDB(SavedGameDataDBPath, SavedGameDataJsonDB, SavedGameDataFieldName, true))
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("ActorStateDB 初始化成功！"));
			print(TEXT("ActorStateDB 初始化成功！"));
		}
	}
}


void UWarPersistentSystem::InitInventories()
{
	InventoryFieldName = "Inventories";
	InventoryDBPath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Database/"), TEXT("InventoryDB.json"));
	if (InitJsonDB(InventoryDBPath, InventoryJsonDB, InventoryFieldName, true))
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("InventoryDB 初始化成功！"));
			print(TEXT("InventoryDB 初始化成功！"));
		}
	}
}

bool UWarPersistentSystem::InitJsonDB(const FString& JsonSavePath, TSharedPtr<FJsonObject>& OutJsonObject, const FString& DefaultFieldName, bool bInitAsArray)
{
	// 如果文件不存在，创建一个空 JSON
	if (!FPaths::FileExists(JsonSavePath))
	{
		OutJsonObject = MakeShared<FJsonObject>();

		// 如果指定了 DefaultFieldName，初始化对应字段（数组或对象）
		if (!DefaultFieldName.IsEmpty())
		{
			if (bInitAsArray)
			{
				OutJsonObject->SetArrayField(DefaultFieldName, TArray<TSharedPtr<FJsonValue>>());
			}
			else
			{
				OutJsonObject->SetObjectField(DefaultFieldName, MakeShared<FJsonObject>());
			}
		}
		// 保存初始化的 JSON
		return true;
	}

	// 读取文件
	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *JsonSavePath))
	{
		print_err(TEXT("Failed to read JSON file: %s"), *JsonSavePath);
		return false;
	}
	// 4. 解析 JSON
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	if (!FJsonSerializer::Deserialize(Reader, OutJsonObject) || !OutJsonObject.IsValid())
	{
		print_err(TEXT("Invalid JSON format in file: %s"), *JsonSavePath);
		return false;
	}

	return true;
}


bool UWarPersistentSystem::SaveJsonToFile(const FString& JsonSavePath, const TSharedPtr<FJsonObject>& JsonObject)
{
	if (!JsonObject.IsValid())
	{
		print_err(TEXT("Invalid JSON object"));
		return false;
	}

	// 序列化为字符串
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	if (!FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer))
	{
		print_err(TEXT("JSON序列化失败"));
		return false;
	}

	// 确保目录存在
	const FString DirPath = FPaths::GetPath(JsonSavePath);
	if (!IFileManager::Get().DirectoryExists(*DirPath))
	{
		IFileManager::Get().MakeDirectory(*DirPath, true);
	}

	// 写入文件
	if (!FFileHelper::SaveStringToFile(OutputString, *JsonSavePath))
	{
		print_err(TEXT("文件保存失败: %s"), *JsonSavePath);
		return false;
	}
	return true;
}


void UWarPersistentSystem::InsertSavedActor(const FWarSaveGameData& SaveGameData) const
{
	if (!SavedGameDataJsonDB.IsValid())
	{
		print(TEXT("SavedGameDataJsonDB is invalid"));
		return;
	}
	// 1. 获取已有的 GmeSavedData 数组
	TArray<TSharedPtr<FJsonValue>> GameSavedDataArray;

	if (SavedGameDataJsonDB->HasField(SavedGameDataFieldName))
	{
		// 如果已存在，取出数组
		GameSavedDataArray = SavedGameDataJsonDB->GetArrayField(SavedGameDataFieldName);
	}

	// 2. 检查是否已有相同 InstanceID（去重，防止重复）
	for (const TSharedPtr<FJsonValue>& Value : GameSavedDataArray)
	{
		TSharedPtr<FJsonObject> ExistingActor = Value->AsObject();
		if (ExistingActor.IsValid() && ExistingActor->GetStringField(TEXT("InstanceID")) == SaveGameData.InstanceID.ToString())
		{
			print(TEXT("SaveGameData with InstanceID '%s' already exists."), *SaveGameData.InstanceID.ToString());
			return; // 已存在，直接跳过
		}
	}

	// 3. 创建新的 Actor JSON 对象
	TSharedPtr<FJsonObject> NewActorData = MakeShared<FJsonObject>();
	NewActorData->SetStringField("InstanceID", SaveGameData.InstanceID.ToString());
	NewActorData->SetStringField("TableRowID", SaveGameData.TableRowID.ToString());
	NewActorData->SetStringField("OwnerID", SaveGameData.OwnerID.ToString());
	NewActorData->SetNumberField("Count", SaveGameData.Count);
	NewActorData->SetStringField("ActorClassPath", SaveGameData.ActorClassPath.ToString());
	NewActorData->SetNumberField("LocationX", SaveGameData.Location.X);
	NewActorData->SetNumberField("LocationY", SaveGameData.Location.Y);
	NewActorData->SetNumberField("LocationZ", SaveGameData.Location.Z);
	NewActorData->SetNumberField("RotationPitch", SaveGameData.Rotation.Pitch);
	NewActorData->SetNumberField("RotationYaw", SaveGameData.Rotation.Yaw);
	NewActorData->SetNumberField("RotationRoll", SaveGameData.Rotation.Roll);
	NewActorData->SetNumberField("ScaleX", SaveGameData.Scale.X);
	NewActorData->SetNumberField("ScaleY", SaveGameData.Scale.Y);
	NewActorData->SetNumberField("ScaleZ", SaveGameData.Scale.Z);
	NewActorData->SetStringField("ActorData", FBase64::Encode(SaveGameData.ActorData));
	NewActorData->SetBoolField("bIsDynamicActor", SaveGameData.bIsDynamicActor);
	NewActorData->SetBoolField("bIsDestroyed", SaveGameData.bIsDestroyed);

	// 4. 添加到数组
	GameSavedDataArray.Add(MakeShared<FJsonValueObject>(NewActorData));

	// 5. 更新 JSON 数据库
	SavedGameDataJsonDB->SetArrayField(SavedGameDataFieldName, GameSavedDataArray);
}


void UWarPersistentSystem::UpdateSavedActor(const FWarSaveGameData& SaveGameData) const
{
	if (!SavedGameDataJsonDB.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("SavedGameDataJsonDB is invalid"));
		return;
	}

	// 1. 获取 JSON 数组
	TArray<TSharedPtr<FJsonValue>> GameSavedDataArray;
	if (SavedGameDataJsonDB->HasField(SavedGameDataFieldName))
	{
		GameSavedDataArray = SavedGameDataJsonDB->GetArrayField(SavedGameDataFieldName);
	}

	// 2. 遍历并查找是否存在对应 InstanceID
	for (TSharedPtr<FJsonValue>& Value : GameSavedDataArray)
	{
		TSharedPtr<FJsonObject> ActorObject = Value->AsObject();
		if (ActorObject.IsValid())
		{
			FString ID;
			if (ActorObject->TryGetStringField(TEXT("InstanceID"), ID) && ID == SaveGameData.InstanceID.ToString())
			{
				// 3. 更新字段
				ActorObject->SetNumberField("Count", SaveGameData.Count);
				ActorObject->SetStringField("ActorClassPath", SaveGameData.ActorClassPath.ToString());
				ActorObject->SetStringField("TableRowID", SaveGameData.TableRowID.ToString());
				ActorObject->SetStringField("OwnerID", SaveGameData.OwnerID.ToString());
				ActorObject->SetNumberField("LocationX", SaveGameData.Location.X);
				ActorObject->SetNumberField("LocationY", SaveGameData.Location.Y);
				ActorObject->SetNumberField("LocationZ", SaveGameData.Location.Z);
				ActorObject->SetNumberField("RotationPitch", SaveGameData.Rotation.Pitch);
				ActorObject->SetNumberField("RotationYaw", SaveGameData.Rotation.Yaw);
				ActorObject->SetNumberField("RotationRoll", SaveGameData.Rotation.Roll);
				ActorObject->SetNumberField("ScaleX", SaveGameData.Scale.X);
				ActorObject->SetNumberField("ScaleY", SaveGameData.Scale.Y);
				ActorObject->SetNumberField("ScaleZ", SaveGameData.Scale.Z);
				ActorObject->SetStringField("ActorData", FBase64::Encode(SaveGameData.ActorData));
				ActorObject->SetBoolField("bIsDynamicActor", SaveGameData.bIsDynamicActor);
				ActorObject->SetBoolField("bIsDestroyed", SaveGameData.bIsDestroyed);
				break;
			}
		}
	}

	// 5. 写回 JSON 数据
	SavedGameDataJsonDB->SetArrayField(SavedGameDataFieldName, GameSavedDataArray);
}

bool UWarPersistentSystem::FindAllSavedActors(TArray<FWarSaveGameData>& OutResult) const
{
	if (!SavedGameDataJsonDB.IsValid())
	{
		print(TEXT("SavedGameDataJsonDB is invalid."));
		return false;
	}
	// 1. 检查是否存在 "ActorStates" 数组字段
	if (!SavedGameDataJsonDB->HasTypedField<EJson::Array>(SavedGameDataFieldName))
	{
		print(TEXT("No SavedGameDataFieldName found in the JSON DB."));
		return false;
	}

	if (SavedGameDataJsonDB->GetArrayField(SavedGameDataFieldName).IsEmpty())
	{
		print(TEXT("No SavedGame items found in the JSON DB."));
		return false;
	}

	// 2. 获取 JSON 数组
	const TArray<TSharedPtr<FJsonValue>>& SavedActorArray = SavedGameDataJsonDB->GetArrayField(SavedGameDataFieldName);

	bool bFoundAny = false;

	// 3. 遍历 JSON 数组，查找指定 ActorName
	for (const TSharedPtr<FJsonValue>& JsonValue : SavedActorArray)
	{
		TSharedPtr<FJsonObject> ActorObject = JsonValue->AsObject();
		if (ActorObject.IsValid())
		{
			FWarSaveGameData SavedActorInDB;
			SavedActorInDB.InstanceID = FGuid(ActorObject->GetStringField(TEXT("InstanceID")));
			SavedActorInDB.ActorClassPath = ActorObject->GetStringField(TEXT("ActorClassPath"));
			SavedActorInDB.TableRowID = FName(*ActorObject->GetStringField(TEXT("TableRowID")));
			SavedActorInDB.OwnerID = FGuid(ActorObject->GetStringField(TEXT("OwnerID")));
			SavedActorInDB.Count = ActorObject->GetIntegerField(TEXT("Count"));
			SavedActorInDB.bIsDestroyed = ActorObject->GetBoolField(TEXT("bIsDestroyed"));
			SavedActorInDB.bIsDynamicActor = ActorObject->GetBoolField(TEXT("bIsDynamicActor"));
			SavedActorInDB.Location.X = ActorObject->GetIntegerField(TEXT("LocationX"));
			SavedActorInDB.Location.Y = ActorObject->GetIntegerField(TEXT("LocationY"));
			SavedActorInDB.Location.Z = ActorObject->GetIntegerField(TEXT("LocationZ"));
			SavedActorInDB.Rotation.Pitch = ActorObject->GetIntegerField(TEXT("RotationPitch"));
			SavedActorInDB.Rotation.Yaw = ActorObject->GetIntegerField(TEXT("RotationYaw"));
			SavedActorInDB.Scale.X = ActorObject->GetIntegerField(TEXT("ScaleX"));
			SavedActorInDB.Scale.Y = ActorObject->GetIntegerField(TEXT("ScaleY"));
			SavedActorInDB.Scale.Z = ActorObject->GetIntegerField(TEXT("ScaleZ"));

			TArray<uint8> ActorData = TArray<uint8>();
			FString StringToBase64 = ActorObject->GetStringField(TEXT("ActorData"));
			if (StringToBase64.Len() > 0)
			{
				FBase64::Decode(StringToBase64, ActorData);
			}
			SavedActorInDB.ActorData = ActorData;
			OutResult.Add(SavedActorInDB);
			bFoundAny = true;
		}
	}
	return bFoundAny;
}


bool UWarPersistentSystem::RemoveSavedActorByID(const FGuid& InInstanceID) const
{
	if (!InInstanceID.IsValid()) return false;

	if (!SavedGameDataJsonDB.IsValid() || !SavedGameDataJsonDB->HasTypedField<EJson::Array>(SavedGameDataFieldName))
	{
		return false;
	}

	TArray<TSharedPtr<FJsonValue>> ActorArray = SavedGameDataJsonDB->GetArrayField(SavedGameDataFieldName);

	bool bSuccess = false;

	for (int32 i = ActorArray.Num() - 1; i >= 0; --i) // 逆序删除
	{
		TSharedPtr<FJsonObject> ActorObject = ActorArray[i]->AsObject();
		FGuid IDGuid;
		if (WarJsonHelper::ExtractInventoryOnlyID(ActorObject, IDGuid) && IDGuid == InInstanceID)
		{
			ActorArray.RemoveAt(i);
			print(TEXT("已移除存档中的 Actor: %s"), *InInstanceID.ToString());
			bSuccess = true;
			break;
		}
	}
	SavedGameDataJsonDB->SetArrayField(SavedGameDataFieldName, ActorArray);
	return bSuccess;
}

bool UWarPersistentSystem::MarkAsDestroyed(const FGuid& InInstanceID) const
{
	if (!InInstanceID.IsValid()) return false;

	if (!SavedGameDataJsonDB.IsValid() || !SavedGameDataJsonDB->HasTypedField<EJson::Array>(SavedGameDataFieldName))
	{
		return false;
	}

	TArray<TSharedPtr<FJsonValue>> ActorArray = SavedGameDataJsonDB->GetArrayField(SavedGameDataFieldName);
	bool bSuccess = false;

	for (const auto& Item : ActorArray)
	{
		TSharedPtr<FJsonObject> ActorObject = Item->AsObject();
		FGuid IDGuid;
		if (WarJsonHelper::ExtractInventoryOnlyID(ActorObject, IDGuid) && IDGuid == InInstanceID)
		{
			ActorObject->SetBoolField("bIsDestroyed", true);
			bSuccess = true;
			break; // 一旦找到就退出
		}
	}
	SavedGameDataJsonDB->SetArrayField(SavedGameDataFieldName, ActorArray);
	return bSuccess;
}

/*
 * PlayerID 和 TableID确定数据库中是否存在记录。
 * 如果是可堆叠的物品，并且存在记录就将Count+1, 更新InstanceID覆盖掉原始InstanceID。
 */
void UWarPersistentSystem::InsertInventory(const FInventoryItemInDB& InventoryItemInDB) const
{
	if (!InventoryJsonDB.IsValid())
	{
		print(TEXT("InventoryJsonDB is invalid"));
		return;
	}

	TArray<TSharedPtr<FJsonValue>> InventoryArray;
	if (InventoryJsonDB->HasField(InventoryFieldName))
	{
		InventoryArray = InventoryJsonDB->GetArrayField(InventoryFieldName);
	}

	bool bIsStackable = false;

	switch (InventoryItemInDB.InventoryType)
	{
	case EWarInventoryType::QuestItem:
	case EWarInventoryType::Consumable:
	case EWarInventoryType::Material:
	case EWarInventoryType::None:
		bIsStackable = true;
		break;
	default:
		break;
	}

	// 处理堆叠物品
	if (bIsStackable)
	{
		for (const TSharedPtr<FJsonValue>& Value : InventoryArray)
		{
			TSharedPtr<FJsonObject> ExistingInventory = Value->AsObject();
			FGuid PlayerIDGuid;
			FName TableRowID;
			// 可堆叠物品
			if (WarJsonHelper::ExtractInventoryWithTable(ExistingInventory, PlayerIDGuid, TableRowID) && PlayerIDGuid == InventoryItemInDB.PlayerID && TableRowID == InventoryItemInDB.TableRowID)
			{
				// 找到已有物品，直接堆叠数量
				int32 OldCount = ExistingInventory->GetIntegerField(TEXT("Count"));
				ExistingInventory->SetNumberField(TEXT("Count"), OldCount + InventoryItemInDB.Count);
				// ✅ 覆盖 InstanceID
				ExistingInventory->SetStringField(TEXT("InstanceID"), InventoryItemInDB.InstanceID.ToString());
				// 写回数据库
				InventoryJsonDB->SetArrayField(InventoryFieldName, InventoryArray);
				print(TEXT("堆叠物品成功: %s，当前数量: %d"), *InventoryItemInDB.TableRowID.ToString(), OldCount + InventoryItemInDB.Count);
				return;
			}
		}
	}

	// 不可堆叠物品 or 没找到堆叠目标，创建新对象
	TSharedPtr<FJsonObject> NewData = MakeShared<FJsonObject>();
	NewData->SetStringField("PlayerID", InventoryItemInDB.PlayerID.ToString());
	NewData->SetStringField("TableRowID", InventoryItemInDB.TableRowID.ToString());
	NewData->SetNumberField("InventoryType", static_cast<int32>(InventoryItemInDB.InventoryType));
	NewData->SetNumberField("Amount", InventoryItemInDB.Amount);
	NewData->SetNumberField("Cooldown", InventoryItemInDB.Cooldown);
	NewData->SetNumberField("Count", InventoryItemInDB.Count);
	NewData->SetBoolField("bIsEquipped", InventoryItemInDB.bIsEquipped);
	NewData->SetNumberField("Damage", InventoryItemInDB.Damage);
	NewData->SetNumberField("Defense", InventoryItemInDB.Defense);
	NewData->SetNumberField("Durability", InventoryItemInDB.Durability);
	NewData->SetNumberField("Level", InventoryItemInDB.Level);
	NewData->SetStringField("QuestID", InventoryItemInDB.QuestID.ToString());
	NewData->SetStringField("InstanceID", InventoryItemInDB.InstanceID.ToString());
	InventoryArray.Add(MakeShared<FJsonValueObject>(NewData));
	InventoryJsonDB->SetArrayField(InventoryFieldName, InventoryArray);
	print(TEXT("新物品已添加: %s"), *InventoryItemInDB.TableRowID.ToString());
}


void UWarPersistentSystem::InsertInventoryWithMax(const FInventoryItemInDB& InventoryItemInDB) const
{
	int32 MaxSlot = UWarGameInstanceSubSystem::GetGameConfigData(this)->MaxSlots;
	if (MaxSlot <= 0) return;

	if (FindAllInventoriesCount(InventoryItemInDB.PlayerID) < MaxSlot)
	{
		InsertInventory(InventoryItemInDB);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("背包已满无法添加物品.."));
		print(TEXT("背包已满无法添加物品.."));
	}
}

void UWarPersistentSystem::UpdateInventory(const FInventoryItemInDB& InventoryItemInDB) const
{
	if (!InventoryJsonDB.IsValid())
	{
		print(TEXT("InventoryJsonDB is invalid"));
		return;
	}

	if (!InventoryJsonDB->HasField(InventoryFieldName))
	{
		print(TEXT("Inventory list is empty, update failed."));
		return;
	}

	TArray<TSharedPtr<FJsonValue>> InventoryArray = InventoryJsonDB->GetArrayField(InventoryFieldName);

	for (const TSharedPtr<FJsonValue>& Value : InventoryArray)
	{
		TSharedPtr<FJsonObject> ExistingInventory = Value->AsObject();
		FGuid IDGuid;
		if (WarJsonHelper::ExtractInventoryOnlyID(ExistingInventory, IDGuid) && IDGuid == InventoryItemInDB.InstanceID)
		{
			// ✅ 找到目标，更新所有字段
			ExistingInventory->SetStringField("PlayerID", InventoryItemInDB.PlayerID.ToString());
			ExistingInventory->SetStringField("TableRowID", InventoryItemInDB.TableRowID.ToString());
			ExistingInventory->SetNumberField("InventoryType", static_cast<int32>(InventoryItemInDB.InventoryType));
			ExistingInventory->SetNumberField("Amount", InventoryItemInDB.Amount);
			ExistingInventory->SetNumberField("Cooldown", InventoryItemInDB.Cooldown);
			ExistingInventory->SetNumberField("Count", InventoryItemInDB.Count);
			ExistingInventory->SetNumberField("Damage", InventoryItemInDB.Damage);
			ExistingInventory->SetBoolField("bIsEquipped", InventoryItemInDB.bIsEquipped);
			ExistingInventory->SetNumberField("Defense", InventoryItemInDB.Defense);
			ExistingInventory->SetNumberField("Durability", InventoryItemInDB.Durability);
			ExistingInventory->SetNumberField("Level", InventoryItemInDB.Level);
			ExistingInventory->SetStringField("QuestID", InventoryItemInDB.QuestID.ToString());

			// 可选：更新时间戳字段，做版本控制
			// ExistingInventory->SetStringField("LastModified", FDateTime::UtcNow().ToString());

			// 写回数据库
			InventoryJsonDB->SetArrayField(InventoryFieldName, InventoryArray);

			print(TEXT("物品已更新: %s"), *InventoryItemInDB.InstanceID.ToString());
			return;
		}
	}
	print(TEXT("未找到需要更新的物品: %s"), *InventoryItemInDB.InstanceID.ToString());
}

//读写操作
void UWarPersistentSystem::MarkAsEquipped(const FGuid& InventoryID, const FGuid& PlayerID) const
{
	FInventoryItemInDB OldItem;
	if (FindInventoryByID(InventoryID, PlayerID, OldItem))
	{
		OldItem.bIsEquipped = true;
		UpdateInventory(OldItem);
	}
}

void UWarPersistentSystem::MarkAsUnEquipped(const FGuid& InventoryID, const FGuid& PlayerID) const
{
	FInventoryItemInDB OldItem;
	if (FindInventoryByID(InventoryID, PlayerID, OldItem))
	{
		OldItem.bIsEquipped = false;
		UpdateInventory(OldItem);
	}
}

bool UWarPersistentSystem::FindInventoryByState(const FGuid& PlayerID, const bool bEquippedState, TArray<FInventoryItemInDB>& OutResult) const
{
	if (!PlayerID.IsValid()) return false;

	if (!InventoryJsonDB.IsValid())
	{
		print(TEXT("InventoryJsonDB is invalid."));
		return false;
	}
	// 1. 检查是否存在 "Inventories" 数组字段
	if (!InventoryJsonDB->HasTypedField<EJson::Array>(InventoryFieldName))
	{
		print(TEXT("No Inventories found in the JSON DB."));
		return false;
	}

	// 2. 获取 JSON 数组
	const TArray<TSharedPtr<FJsonValue>>& InventoryArray = InventoryJsonDB->GetArrayField(InventoryFieldName);
	bool bFoundAny = false;

	// 3. 遍历 JSON 数组，查找指定 ActorName
	for (const TSharedPtr<FJsonValue>& JsonValue : InventoryArray)
	{
		TSharedPtr<FJsonObject> InventoryObject = JsonValue->AsObject();
		FGuid PlayerIDGuid;
		bool bIsEquipped = false;

		if (WarJsonHelper::ExtractEquippedInventoryKeys(InventoryObject, PlayerIDGuid, bIsEquipped) && PlayerIDGuid == PlayerID && bIsEquipped == bEquippedState)
		{
			FInventoryItemInDB NewData;
			NewData.InstanceID = FGuid(InventoryObject->GetStringField(TEXT("InstanceID")));
			NewData.InventoryType = static_cast<EWarInventoryType>(InventoryObject->GetNumberField(TEXT("InventoryType")));
			NewData.PlayerID = FGuid(InventoryObject->GetStringField(TEXT("PlayerID")));
			NewData.TableRowID = FName(*InventoryObject->GetStringField(TEXT("TableRowID")));
			NewData.Damage = InventoryObject->GetNumberField(TEXT("Damage"));
			NewData.Defense = InventoryObject->GetNumberField(TEXT("Defense"));
			NewData.Amount = InventoryObject->GetNumberField(TEXT("Amount"));
			NewData.bIsEquipped = InventoryObject->GetBoolField(TEXT("bIsEquipped"));
			NewData.Cooldown = InventoryObject->GetNumberField(TEXT("Cooldown"));
			NewData.Count = InventoryObject->GetNumberField(TEXT("Count"));
			NewData.Durability = InventoryObject->GetNumberField(TEXT("Durability"));
			NewData.Level = InventoryObject->GetNumberField(TEXT("Level"));
			NewData.QuestID = FGuid(InventoryObject->GetStringField(TEXT("QuestID")));

			OutResult.Add(NewData);
			bFoundAny = true;
		}
	}
	return bFoundAny;
}

bool UWarPersistentSystem::HasInventory(const FGuid& InventoryID, const FGuid& PlayerID) const
{
	if (!InventoryID.IsValid() || !PlayerID.IsValid()) return false;
	if (!InventoryJsonDB.IsValid()) return false;
	if (!InventoryJsonDB->HasField(InventoryFieldName)) return false;

	const TArray<TSharedPtr<FJsonValue>> InventoryArray = InventoryJsonDB->GetArrayField(InventoryFieldName);

	for (const TSharedPtr<FJsonValue>& Value : InventoryArray)
	{
		const TSharedPtr<FJsonObject> Obj = Value->AsObject();
		FGuid IDGuid, PlayerIDGuid;
		if (WarJsonHelper::ExtractInventoryKeys(Obj, IDGuid, PlayerIDGuid) && IDGuid == InventoryID && PlayerIDGuid == PlayerID)
		{
			return true;
		}
	}
	return false;
}

bool UWarPersistentSystem::HasEquipped(const FGuid& InventoryID, const FGuid& PlayerID) const
{
	if (!InventoryID.IsValid() || !PlayerID.IsValid()) return false;
	if (!InventoryJsonDB.IsValid()) return false;
	if (!InventoryJsonDB->HasField(InventoryFieldName)) return false;

	const TArray<TSharedPtr<FJsonValue>> InventoryArray = InventoryJsonDB->GetArrayField(InventoryFieldName);

	for (const TSharedPtr<FJsonValue>& Value : InventoryArray)
	{
		const TSharedPtr<FJsonObject> Obj = Value->AsObject();
		FGuid IDGuid, PlayerIDGuid;
		if (WarJsonHelper::ExtractInventoryKeys(Obj, IDGuid, PlayerIDGuid) && IDGuid == InventoryID && PlayerIDGuid == PlayerID)
		{
			return true;
		}
	}
	return false;
}


bool UWarPersistentSystem::FindInventoryByID(const FGuid& InventoryID, const FGuid& PlayerID, FInventoryItemInDB& OutResult) const
{
	if (!InventoryID.IsValid() || !PlayerID.IsValid()) return false;

	if (!InventoryJsonDB.IsValid())
	{
		print(TEXT("InventoryJsonDB is invalid."));
		return false;
	}
	// 1. 检查是否存在 "Inventories" 数组字段
	if (!InventoryJsonDB->HasTypedField<EJson::Array>(InventoryFieldName))
	{
		print(TEXT("No Inventories found in the JSON DB."));
		return false;
	}

	// 2. 获取 JSON 数组
	const TArray<TSharedPtr<FJsonValue>>& InventoryArray = InventoryJsonDB->GetArrayField(InventoryFieldName);

	// 3. 遍历 JSON 数组，查找指定 ActorName
	for (const TSharedPtr<FJsonValue>& JsonValue : InventoryArray)
	{
		TSharedPtr<FJsonObject> InventoryObject = JsonValue->AsObject();

		FGuid IDGuid, PlayerIDGuid;
		if (WarJsonHelper::ExtractInventoryKeys(InventoryObject, IDGuid, PlayerIDGuid) && IDGuid == InventoryID && PlayerIDGuid == PlayerID)
		{
			// 映射 JSON 到 FActorStateInDB 结构体
			OutResult.InstanceID = FGuid(InventoryObject->GetStringField(TEXT("InstanceID")));
			OutResult.InventoryType = static_cast<EWarInventoryType>(InventoryObject->GetNumberField(TEXT("InventoryType")));
			OutResult.PlayerID = FGuid(InventoryObject->GetStringField(TEXT("PlayerID")));
			OutResult.TableRowID = FName(*InventoryObject->GetStringField(TEXT("TableRowID")));
			OutResult.Damage = InventoryObject->GetNumberField(TEXT("Damage"));
			OutResult.Defense = InventoryObject->GetNumberField(TEXT("Defense"));
			OutResult.Amount = InventoryObject->GetNumberField(TEXT("Amount"));
			OutResult.bIsEquipped = InventoryObject->GetBoolField(TEXT("bIsEquipped"));
			OutResult.Cooldown = InventoryObject->GetNumberField(TEXT("Cooldown"));
			OutResult.Count = InventoryObject->GetNumberField(TEXT("Count"));
			OutResult.Durability = InventoryObject->GetNumberField(TEXT("Durability"));
			OutResult.Level = InventoryObject->GetNumberField(TEXT("Level"));
			OutResult.QuestID = FGuid(InventoryObject->GetStringField(TEXT("QuestID")));
			return true;
		}
	}
	return false;
}


bool UWarPersistentSystem::FindAllInventoriesByPlayerID(const FGuid& InPlayerID, TArray<FInventoryItemInDB>& OutResult) const
{
	if (!InventoryJsonDB.IsValid())
	{
		print(TEXT("InventoryJsonDB is invalid."));
		return false;
	}
	// 1. 检查是否存在 "Inventories" 数组字段
	if (!InventoryJsonDB->HasTypedField<EJson::Array>(InventoryFieldName))
	{
		print(TEXT("No Inventories found in the JSON DB."));
		return false;
	}

	// 2. 获取 JSON 数组
	const TArray<TSharedPtr<FJsonValue>>& InventoryArray = InventoryJsonDB->GetArrayField(InventoryFieldName);

	bool bFoundAny = false;

	// 3. 遍历 JSON 数组，查找指定 ActorName
	for (const TSharedPtr<FJsonValue>& JsonValue : InventoryArray)
	{
		TSharedPtr<FJsonObject> InventoryObject = JsonValue->AsObject();

		FGuid PlayerIDGuid;
		if (WarJsonHelper::ExtractInventoryOnlyPlayerID(InventoryObject, PlayerIDGuid) && PlayerIDGuid == InPlayerID)
		{
			FInventoryItemInDB NewData;

			NewData.InstanceID = FGuid(InventoryObject->GetStringField(TEXT("InstanceID")));
			NewData.InventoryType = static_cast<EWarInventoryType>(InventoryObject->GetNumberField(TEXT("InventoryType")));
			NewData.PlayerID = FGuid(InventoryObject->GetStringField(TEXT("PlayerID")));
			NewData.TableRowID = FName(*InventoryObject->GetStringField(TEXT("TableRowID")));
			NewData.Damage = InventoryObject->GetNumberField(TEXT("Damage"));
			NewData.Defense = InventoryObject->GetNumberField(TEXT("Defense"));
			NewData.Amount = InventoryObject->GetNumberField(TEXT("Amount"));
			NewData.bIsEquipped = InventoryObject->GetBoolField(TEXT("bIsEquipped"));
			NewData.Cooldown = InventoryObject->GetNumberField(TEXT("Cooldown"));
			NewData.Count = InventoryObject->GetNumberField(TEXT("Count"));
			NewData.Durability = InventoryObject->GetNumberField(TEXT("Durability"));
			NewData.Level = InventoryObject->GetNumberField(TEXT("Level"));
			NewData.QuestID = FGuid(InventoryObject->GetStringField(TEXT("QuestID")));

			OutResult.Add(NewData);
			bFoundAny = true;
		}
	}
	return bFoundAny;
}

int32 UWarPersistentSystem::FindAllInventoriesCount(const FGuid& InPlayerID) const
{
	if (!InventoryJsonDB.IsValid() || !InPlayerID.IsValid()) return 0;

	const TArray<TSharedPtr<FJsonValue>>* InventoryArrayPtr;
	if (!InventoryJsonDB->TryGetArrayField(InventoryFieldName, InventoryArrayPtr)) return 0;

	int32 Count = 0;
	for (const auto& Value : *InventoryArrayPtr)
	{
		const TSharedPtr<FJsonObject> Obj = Value->AsObject();
		if (!Obj.IsValid()) continue;

		FString PlayerIDStr;
		if (Obj->TryGetStringField(TEXT("PlayerID"), PlayerIDStr))
		{
			FGuid FoundID;
			if (FGuid::Parse(PlayerIDStr, FoundID) && FoundID == InPlayerID)
			{
				++Count;
			}
		}
	}
	return Count;
}


void UWarPersistentSystem::SaveGame()
{
	// 每次重新扫描，确保兼容动态生成和销毁
	TArray<AActor*> SaveActors;

	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		AActor* Actor = *It;
		if (IWarSaveGameInterface* SaveInterface = Cast<IWarSaveGameInterface>(Actor))
		{
			SaveActors.Add(Actor);
			// 如果 Actor 已经有 ID，才构造更新数据
			if (!SaveInterface->GetPersistentID().IsValid()) return;

			//存档数据构造
			FWarSaveGameData SavedGameData;
			SavedGameData.bIsDestroyed = false;
			SavedGameData.InstanceID = SaveInterface->GetPersistentID();
			SavedGameData.TableRowID = SaveInterface->GetTableRowID();
			// 保存时，区分动态和静态
			SavedGameData.bIsDynamicActor = Actor->ActorHasTag(TEXT("DynamicActor"));
			if (SavedGameData.bIsDynamicActor)
			{
				SavedGameData.ActorClassPath = Actor->GetClass(); // 仅动态物体记录 Class
			}
			else
			{
				SavedGameData.ActorClassPath = nullptr;
			}

			if (AInventoryBase* Inventory = Cast<AInventoryBase>(Actor))
			{
				if (Inventory->GetCurrentWorldState() == EInventoryWorldState::Equipped)
				{
					AActor* OwnerActor = Inventory->GetAttachParentActor();
					if (!OwnerActor) return;
					if (AWarCharacterBase* OwnerCharacter = Cast<AWarCharacterBase>(OwnerActor))
					{
						SavedGameData.OwnerID = OwnerCharacter->GetPersistentID();
					}
				}
			}

			SavedGameData.Location = Actor->GetActorLocation();
			SavedGameData.Rotation = Actor->GetActorRotation();
			SavedGameData.Scale = Actor->GetActorScale();

			//TODO:写入内存
			UpdateSavedActor(SavedGameData);
		}
	}
	// ⚙️ 存档时覆盖 JSON，防止历史数据残留
	SaveJsonToFile(SavedGameDataDBPath, SavedGameDataJsonDB); // true: 清理后写入
	SaveJsonToFile(InventoryDBPath, InventoryJsonDB);
}


void UWarPersistentSystem::LoadGame()
{
	//重新读取装备数据
	InitInventories();

	TArray<FWarSaveGameData> SaveGameDataArray;
	bool bSuccess = FindAllSavedActors(SaveGameDataArray);
	if (!bSuccess)
	{
		print(TEXT("FindAllSavedActors 执行失败！"))
		return;
	}

	// 建立快速 Guid 映射表
	TMap<FGuid, AActor*> GuidToActorMap;

	//先快速过滤拿到实现接口的Actors
	for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		AActor* Actor = *ActorItr;
		if (IWarSaveGameInterface* SaveInterface = Cast<IWarSaveGameInterface>(Actor))
		{
			GuidToActorMap.Add(SaveInterface->GetPersistentID(), Actor);
		}
	}

	//循环遍历次存档中Actors
	for (const FWarSaveGameData& SavedGameData : SaveGameDataArray)
	{
		if (!SavedGameData.InstanceID.IsValid()) return;
		// 恢复 Transform
		FTransform NewTransform;
		NewTransform.SetLocation(SavedGameData.Location);
		NewTransform.SetRotation(SavedGameData.Rotation.Quaternion());
		NewTransform.SetScale3D(SavedGameData.Scale);

		// 快速查找场景内是否存在该 Actor
		if (AActor** FoundActor = GuidToActorMap.Find(SavedGameData.InstanceID))
		{
			AActor* Actor = *FoundActor;
			if (SavedGameData.bIsDestroyed)
			{
				print(TEXT("需要删除的 %s "), *Actor->GetName());
				//已销毁，移除场景
				Actor->Destroy();
				continue;
			}
			Actor->SetActorTransform(NewTransform);
			// 恢复自定义数据(接口里面序列化的子项)
			FMemoryReader MemoryReader(SavedGameData.ActorData, true);
			MemoryReader.Seek(0);
			if (IWarSaveGameInterface* SaveInterface = Cast<IWarSaveGameInterface>(Actor))
			{
				SaveInterface->LoadActorData(MemoryReader);
			}
		}
		else
		{
			// 支持动态生成
			if (!SavedGameData.bIsDynamicActor) return;
			if (!SavedGameData.ActorClassPath.IsValid()) return;

			if (UClass* ActorClass = Cast<UClass>(SavedGameData.ActorClassPath.TryLoad()))
			{
				AActor* NewActor = GetWorld()->SpawnActor<AActor>(ActorClass, NewTransform);
				if (!NewActor) return;

				if (IWarSaveGameInterface* SaveInterface = Cast<IWarSaveGameInterface>(NewActor))
				{
					SaveInterface->SetPersistentID(SavedGameData.InstanceID);
					// 恢复自定义数据(接口里面序列化的子项)
					FMemoryReader MemoryReader(SavedGameData.ActorData, true);
					MemoryReader.Seek(0);
					SaveInterface->LoadActorData(MemoryReader);
				}

				if (AInventoryBase* Inventory = Cast<AInventoryBase>(NewActor))
				{
					RestoreEquippedInventory(Inventory, SavedGameData.OwnerID);
				}
				//生成
				UGameplayStatics::FinishSpawningActor(NewActor, NewTransform);
				print(TEXT("加载 Spawn Actor：%s [%s]"), *NewActor->GetName(), *SavedGameData.InstanceID.ToString());
			}
		}
	}
}


//静态方法，给Actor分配全局ID
void UWarPersistentSystem::GeneratorPersistentID(AActor* Actor)
{
	if (!Actor) return;

	UWorld* World = Actor->GetWorld();
	if (!World) return;

	UWarGameInstanceSubSystem* Subsystem = World->GetGameInstance()->GetSubsystem<UWarGameInstanceSubSystem>();
	if (!Subsystem) return;

	UWarPersistentSystem* PersistentSystem = Subsystem->GetWarPersistentSystem();
	if (!PersistentSystem) return;

	if (!Actor) return;
	if (IWarSaveGameInterface* SaveInterface = Cast<IWarSaveGameInterface>(Actor))
	{
		// 检查是否已分配过 ID，避免重复生成
		if (!SaveInterface->GetPersistentID().IsValid())
		{
			FGuid NewID = FGuid::NewGuid();
			SaveInterface->SetPersistentID(NewID);
			FWarSaveGameData SavedGameData;
			SavedGameData.InstanceID = SaveInterface->GetPersistentID();
			//写入内存
			PersistentSystem->InsertSavedActor(SavedGameData);
		}
	}
}


// PersistentSystem 负责将装备挂载回角色
void UWarPersistentSystem::RestoreEquippedInventory(AInventoryBase* Inventory, const FGuid& PlayerID) const
{
	AWarHeroCharacter* Character = UWarGameInstanceSubSystem::FindCharacterByPersistentID(this, PlayerID);
	if (!Character || !Character->GetMesh()) return;

	const FWarInventoryRow* FindRow = UWarGameInstanceSubSystem::FindInventoryRow(this, Inventory->GetTableRowID());
	if (!FindRow) return;

	FName Socket = FindRow->SocketName;
	Inventory->AttachToComponent(Character->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, Socket);
	print(TEXT("装备 %s 已挂载到 %s 的 Socket %s"), *Inventory->GetName(), *Character->GetName(), *Socket.ToString());
}
