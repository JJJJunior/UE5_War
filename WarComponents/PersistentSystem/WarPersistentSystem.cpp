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


void UWarPersistentSystem::SaveGame()
{
	UWorld* World = GetWorld();
	if (!World) return;

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

void UWarPersistentSystem::LoadGame()
{
	UWarSaveGame* LoadGameInstance = Cast<UWarSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("MySaveSlot"), 0));
	if (!LoadGameInstance) return;

	UWorld* World = GetWorld();
	if (!World) return;

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
	print(TEXT("游戏读取完毕-------------"));
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
