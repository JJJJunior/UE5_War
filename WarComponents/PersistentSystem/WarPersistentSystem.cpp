#include "WarPersistentSystem.h"
#include "EngineUtils.h"
#include "DataManager/DynamicData/InventoryData.h"
#include "GameInstance/WarGameInstanceSubSystem.h"
#include "SaveGame/WarSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Tools/MyLog.h"
#include "SaveGame/Interface/WarSaveGameInterface.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"


UWarPersistentSystem::UWarPersistentSystem()
{
	SavedGameDataFieldName = "SavedActors";
	SavedGameDataDBPath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Database/"), TEXT("SavedGameDataDB.json"));
	if (InitJsonDB(SavedGameDataDBPath, SavedGameDataJsonDB, SavedGameDataFieldName, true))
	{
		// print(TEXT("ActorStateDB 初始化成功！"));
	}

	InventoryFieldName = "Inventories";
	InventoryDBPath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Database/"), TEXT("InventoryDB.json"));
	if (InitJsonDB(InventoryDBPath, InventoryJsonDB, InventoryFieldName, true))
	{
		// print(TEXT("InventoryDB 初始化成功！"));
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
	if (!SavedGameDataJsonDB.IsValid() || !SavedGameDataJsonDB->HasTypedField<EJson::Array>(SavedGameDataFieldName))
	{
		return false;
	}

	TArray<TSharedPtr<FJsonValue>> ActorArray = SavedGameDataJsonDB->GetArrayField(SavedGameDataFieldName);

	bool bSuccess = false;

	for (int32 i = ActorArray.Num() - 1; i >= 0; --i) // 逆序删除
	{
		TSharedPtr<FJsonObject> ActorObject = ActorArray[i]->AsObject();
		if (ActorObject.IsValid() && ActorObject->GetStringField(TEXT("InstanceID")) == InInstanceID.ToString())
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
	if (!SavedGameDataJsonDB.IsValid() || !SavedGameDataJsonDB->HasTypedField<EJson::Array>(SavedGameDataFieldName))
	{
		return false;
	}

	TArray<TSharedPtr<FJsonValue>> ActorArray = SavedGameDataJsonDB->GetArrayField(SavedGameDataFieldName);
	bool bSuccess = false;

	for (const auto& Item : ActorArray)
	{
		TSharedPtr<FJsonObject> ActorObject = Item->AsObject();
		if (!ActorObject.IsValid()) continue;

		FString IDString;
		if (ActorObject->TryGetStringField(TEXT("InstanceID"), IDString) && IDString == InInstanceID.ToString())
		{
			ActorObject->SetBoolField("bIsDestroyed", true);
			bSuccess = true;
			break; // 一旦找到就退出
		}
	}

	SavedGameDataJsonDB->SetArrayField(SavedGameDataFieldName, ActorArray);
	return bSuccess;
}


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
			if (ExistingInventory.IsValid() && ExistingInventory->HasField(TEXT("InstanceID")))
			{
				// 可堆叠物品：InstanceID = TableRowID
				if (ExistingInventory->GetStringField(TEXT("InstanceID")) == InventoryItemInDB.TableRowID.ToString())
				{
					// 找到已有物品，直接堆叠数量
					int32 OldCount = ExistingInventory->GetIntegerField(TEXT("Count"));
					ExistingInventory->SetNumberField(TEXT("Count"), OldCount + InventoryItemInDB.Count);

					// 写回数据库
					InventoryJsonDB->SetArrayField(InventoryFieldName, InventoryArray);
					print(TEXT("堆叠物品成功: %s，当前数量: %d"), *InventoryItemInDB.TableRowID.ToString(), OldCount + InventoryItemInDB.Count);
					return;
				}
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
	NewData->SetNumberField("Damage", InventoryItemInDB.Damage);
	NewData->SetNumberField("Defense", InventoryItemInDB.Defense);
	NewData->SetNumberField("Durability", InventoryItemInDB.Durability);
	NewData->SetNumberField("Level", InventoryItemInDB.Level);
	NewData->SetStringField("QuestID", InventoryItemInDB.QuestID.ToString());

	// 区分堆叠类型的 InstanceID
	switch (InventoryItemInDB.InventoryType)
	{
	case EWarInventoryType::Weapon:
	case EWarInventoryType::Armor:
	case EWarInventoryType::Skill:
		NewData->SetStringField("InstanceID", InventoryItemInDB.InstanceID.ToString());
		break;
	default:
		NewData->SetStringField("InstanceID", InventoryItemInDB.TableRowID.ToString());
		break;
	}

	InventoryArray.Add(MakeShared<FJsonValueObject>(NewData));
	InventoryJsonDB->SetArrayField(InventoryFieldName, InventoryArray);
	print(TEXT("新物品已添加: %s"), *InventoryItemInDB.TableRowID.ToString());
}

bool UWarPersistentSystem::FindInventoryByID(const FGuid& InventoryID, FInventoryItemInDB& OutResult) const
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

	// 3. 遍历 JSON 数组，查找指定 ActorName
	for (const TSharedPtr<FJsonValue>& JsonValue : InventoryArray)
	{
		TSharedPtr<FJsonObject> InventoryObject = JsonValue->AsObject();

		if (InventoryObject.IsValid() && InventoryObject->HasField(TEXT("InstanceID")))
		{
			if (InventoryObject->GetStringField(TEXT("InstanceID")) == InventoryID.ToString())
			{
				// 映射 JSON 到 FActorStateInDB 结构体
				OutResult.InstanceID = FGuid(InventoryObject->GetStringField(TEXT("InstanceID")));
				OutResult.InventoryType = static_cast<EWarInventoryType>(InventoryObject->GetNumberField(TEXT("InventoryType")));
				OutResult.PlayerID = FGuid(InventoryObject->GetStringField(TEXT("PlayerID")));
				OutResult.TableRowID = FName(*InventoryObject->GetStringField(TEXT("TableRowID")));
				OutResult.Damage = InventoryObject->GetNumberField(TEXT("Damage"));
				OutResult.Defense = InventoryObject->GetNumberField(TEXT("Defense"));
				OutResult.Amount = InventoryObject->GetNumberField(TEXT("Amount"));
				OutResult.Cooldown = InventoryObject->GetNumberField(TEXT("Cooldown"));
				OutResult.Count = InventoryObject->GetNumberField(TEXT("Count"));
				OutResult.Durability = InventoryObject->GetNumberField(TEXT("Durability"));
				OutResult.Level = InventoryObject->GetNumberField(TEXT("Level"));
				OutResult.QuestID = FGuid(InventoryObject->GetStringField(TEXT("QuestID")));
				return true;
			}
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

		if (InventoryObject.IsValid() && InventoryObject->HasField(TEXT("PlayerID")))
		{
			if (InventoryObject->GetStringField(TEXT("PlayerID")) == InPlayerID.ToString())
			{
				FInventoryItemInDB NewData;

				NewData.InstanceID = FGuid(InventoryObject->GetStringField(TEXT("InstanceID")));
				NewData.InventoryType = static_cast<EWarInventoryType>(InventoryObject->GetNumberField(TEXT("InventoryType")));
				NewData.PlayerID = FGuid(InventoryObject->GetStringField(TEXT("PlayerID")));
				NewData.TableRowID = FName(*InventoryObject->GetStringField(TEXT("TableRowID")));
				NewData.Damage = InventoryObject->GetNumberField(TEXT("Damage"));
				NewData.Defense = InventoryObject->GetNumberField(TEXT("Defense"));
				NewData.Amount = InventoryObject->GetNumberField(TEXT("Amount"));
				NewData.Cooldown = InventoryObject->GetNumberField(TEXT("Cooldown"));
				NewData.Count = InventoryObject->GetNumberField(TEXT("Count"));
				NewData.Durability = InventoryObject->GetNumberField(TEXT("Durability"));
				NewData.Level = InventoryObject->GetNumberField(TEXT("Level"));
				NewData.QuestID = FGuid(InventoryObject->GetStringField(TEXT("QuestID")));

				OutResult.Add(NewData);
				bFoundAny = true;
			}
		}
	}
	return bFoundAny;
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
			if (SaveInterface->GetPersistentID().IsValid())
			{
				//存档数据构造
				FWarSaveGameData SavedGameData;
				SavedGameData.bIsDestroyed = false;
				SavedGameData.InstanceID = SaveInterface->GetPersistentID();
				// 保存时，区分动态和静态
				SavedGameData.bIsDynamicActor = Actor->ActorHasTag(TEXT("DynamicActor"));
				if (SavedGameData.bIsDynamicActor)
				{
					SavedGameData.ActorClassPath = Actor->GetClass(); // 仅动态物体记录 Class
					print(TEXT("Dynamic Actor Saved: %s"), *Actor->GetName());
				}
				else
				{
					SavedGameData.ActorClassPath = nullptr;
				}

				SavedGameData.Location = Actor->GetActorLocation();
				SavedGameData.Rotation = Actor->GetActorRotation();
				SavedGameData.Scale = Actor->GetActorScale();

				//TODO:写入内存
				UpdateSavedActor(SavedGameData);
			}
		}
	}
	// ⚙️ 存档时覆盖 JSON，防止历史数据残留
	SaveJsonToFile(SavedGameDataDBPath, SavedGameDataJsonDB); // true: 清理后写入
	SaveJsonToFile(InventoryDBPath, InventoryJsonDB);
}


void UWarPersistentSystem::LoadGame()
{
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

	TArray<FWarSaveGameData> SaveGameDataArray;
	FindAllSavedActors(SaveGameDataArray);

	if (SaveGameDataArray.IsEmpty())
	{
		print(TEXT("未找到游戏记录"));
		return;
	}

	//循环遍历次存档中Actors
	for (const FWarSaveGameData& SavedGameData : SaveGameDataArray)
	{
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
			if (SavedGameData.bIsDynamicActor && SavedGameData.ActorClassPath.IsValid())
			{
				if (UClass* ActorClass = Cast<UClass>(SavedGameData.ActorClassPath.TryLoad()))
				{
					AActor* NewActor = GetWorld()->SpawnActor<AActor>(ActorClass, NewTransform);
					if (IWarSaveGameInterface* SaveInterface = Cast<IWarSaveGameInterface>(NewActor))
					{
						SaveInterface->SetPersistentID(SavedGameData.InstanceID);
						// 恢复自定义数据(接口里面序列化的子项)
						FMemoryReader MemoryReader(SavedGameData.ActorData, true);
						MemoryReader.Seek(0);
						SaveInterface->LoadActorData(MemoryReader);
						print(TEXT("加载 Spawn Actor：%s [%s]"), *NewActor->GetName(), *SavedGameData.InstanceID.ToString());
					}
				}
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
