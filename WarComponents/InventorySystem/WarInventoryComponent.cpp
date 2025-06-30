#include "WarInventoryComponent.h"
#include "Characters/Hero/WarHeroCharacter.h"
#include "DataManager/ConfigData/GameConfigData.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Tools/MyLog.h"
#include "GameInstance/WarGameInstanceSubSystem.h"
#include "Kismet/GameplayStatics.h"
#include "War/WorldActors/Inventory/InventoryBase.h"
#include "war/WarComponents/InventorySystem/UI/InventoryPanel/InventoryPanelWidget.h"
#include "War/WarComponents/InventorySystem/UI/RootPanel/RootPanelWidget.h"
#include "WarComponents/PersistentSystem/WarPersistentSystem.h"


UWarInventoryComponent::UWarInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UWarInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	InitRootUI();
	CachedCharacter = Cast<AWarHeroCharacter>(GetOwner());
	if (!CachedCharacter.IsValid()) return;
}

void UWarInventoryComponent::InitRootUI()
{
	UWarGameInstanceSubSystem* Subsystem = UGameplayStatics::GetGameInstance(this)->GetSubsystem<UWarGameInstanceSubSystem>();
	if (!Subsystem) return;
	TSubclassOf<URootPanelWidget> SoftRootPanelClass = Subsystem->GetCachedGameConfigData()->RootPanelWidgetClass;
	checkf(SoftRootPanelClass, TEXT("RootPanelWidget is NULL"));
	RootPanelWidget = CreateWidget<URootPanelWidget>(GetWorld(), Subsystem->GetCachedGameConfigData()->RootPanelWidgetClass);
	// AddToViewport(主视图)
	RootPanelWidget->AddToViewport();
}


//打开或者关闭背包
void UWarInventoryComponent::ToggleInventoryUI()
{
	bInventoryUIVisible = !bInventoryUIVisible;
	NotifyUIStateChanged();
	//背包数据
	if (!SyncJsonToBag())
	{
		print_err(TEXT("SyncJsonToBag() 背包数据同步失败"));
	}
	if (bInventoryUIVisible)
	{
		RootPanelWidget->InventoryPanelWidget->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		RootPanelWidget->InventoryPanelWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}


//根据名称生成装备到人物身上(ok)
void UWarInventoryComponent::SpawnInventory(const FGuid& InInstanceID)
{
	if (!InInstanceID.IsValid()) return;

	UWarGameInstanceSubSystem* Subsystem = UGameplayStatics::GetGameInstance(this)->GetSubsystem<UWarGameInstanceSubSystem>();
	if (!Subsystem) return;

	UWarPersistentSystem* PersistentSystem = Subsystem->GetWarPersistentSystem();
	if (!PersistentSystem) return;

	FInventoryItemInDB InventoryItemDB;
	PersistentSystem->FindInventoryByID(InInstanceID, CachedCharacter->GetPersistentID(), InventoryItemDB);

	//非装备类型不能生成
	if (InventoryItemDB.InventoryType != EWarInventoryType::Armor && InventoryItemDB.InventoryType != EWarInventoryType::Weapon) return;

	AWarHeroCharacter* Character = Cast<AWarHeroCharacter>(GetOuter());
	if (!Character) return;

	//异步生成装备加载class的武器类
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = Character;
	SpawnParameters.Instigator = Character;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	SpawnParameters.TransformScaleMethod = ESpawnActorScaleMethod::MultiplyWithRoot;
	FVector SpawnLocation = FVector::ZeroVector;
	FRotator SpawnRotation = FRotator::ZeroRotator;

	const FWarInventoryRow* ItemRow = UWarGameInstanceSubSystem::FindInventoryRow(this, InventoryItemDB.TableRowID);
	if (!ItemRow)
	{
		print(TEXT("ItemRow 没找到 %s"), *InventoryItemDB.TableRowID.ToString());
		return;
	}

	if (!ItemRow->InventorySoftClass.IsNull())
	{
		FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
		Streamable.RequestAsyncLoad(ItemRow->InventorySoftClass.ToSoftObjectPath(), [this,SpawnLocation, SpawnRotation,SpawnParameters,ItemRow,Character,InInstanceID]()
		{
			UClass* LoadedClass = ItemRow->InventorySoftClass.Get();
			if (!LoadedClass)
			{
				print(TEXT("UWarInventoryComponent::SpawnInventory | LoadedClass is nullptr"));
				return;
			}
			TObjectPtr<AInventoryBase> InventoryActor = GetWorld()->SpawnActor<AInventoryBase>(LoadedClass, SpawnLocation, SpawnRotation, SpawnParameters);
			if (!IsValid(InventoryActor))
			{
				print_err(TEXT("装备InventoryActor 指针丢失"));
				return;
			}
			//消除interaction的碰撞体
			InventoryActor->SetWorldState(EInventoryWorldState::Equipped);
			//移入socket
			if (InventoryActor->AttachToComponent(Character->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, ItemRow->SocketName))
			{
				// 缓存指针
				SavedInventoryInSlots.Emplace(InInstanceID, InventoryActor);
				// for (const auto& Item : SavedInventoryInSlots)
				// {
				// 	print(TEXT("当前SavedInventoryInSlots保存了：%s"), *Item.Key.ToString());
				// }
			}
		});
	}
	else
	{
		UClass* LoadedClass = ItemRow->InventorySoftClass.Get();
		if (!LoadedClass)
		{
			print_err(TEXT("LoadedClass is nullptr"));
			return;
		}
		//生成世界物品
		TObjectPtr<AInventoryBase> InventoryActor = GetWorld()->SpawnActor<AInventoryBase>(LoadedClass, SpawnLocation, SpawnRotation, SpawnParameters);
		if (!IsValid(InventoryActor))
		{
			print_err(TEXT("装备InventoryActor 指针丢失"));
			return;
		}
		//消除interaction的碰撞体
		InventoryActor->SetWorldState(EInventoryWorldState::Equipped);
		//移入socket
		if (InventoryActor->AttachToComponent(Character->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, ItemRow->SocketName))
		{
			// 缓存指针
			SavedInventoryInSlots.Emplace(InInstanceID, InventoryActor);
			// for (const auto& Item : SavedInventoryInSlots)
			// {
			// 	print(TEXT("当前SavedInventoryInSlots保存了：%s"), *Item.Key.ToString());
			// }
		}
	}
}


void UWarInventoryComponent::RollbackSpawnInventory(const FGuid& InInstanceID)
{
	if (SavedInventoryInSlots.Num() <= 0)return;

	TWeakObjectPtr<AInventoryBase> Inventory = FindSavedInventoryInSlots(InInstanceID);
	if (Inventory.IsValid())
	{
		DestroyEquippedInventory(Inventory);
		SavedInventoryInSlots.Remove(InInstanceID);
	}
}

//从背包拿装备穿身上
void UWarInventoryComponent::EquipInventory(const FGuid& InInstanceID)
{
	if (!InInstanceID.IsValid()) return;
	UWarGameInstanceSubSystem* Subsystem = UGameplayStatics::GetGameInstance(this)->GetSubsystem<UWarGameInstanceSubSystem>();
	if (!Subsystem) return;

	UWarPersistentSystem* PersistentSystem = Subsystem->GetWarPersistentSystem();
	if (!PersistentSystem) return;

	if (!PersistentSystem->HasInventory(InInstanceID, CachedCharacter->GetPersistentID())) return;

	FInventoryItemInDB ItemInDB;
	PersistentSystem->FindInventoryByID(InInstanceID, CachedCharacter->GetPersistentID(), ItemInDB);

	//不是装备就略过
	if (ItemInDB.InventoryType != EWarInventoryType::Armor && ItemInDB.InventoryType != EWarInventoryType::Weapon) return;

	//防止崩溃
	if (!CachedCharacter.IsValid() || !CachedCharacter->GetMesh())
	{
		return;
	}

	//生成物品
	SpawnInventory(InInstanceID);
	//将数据标记为已装备
	PersistentSystem->MarkAsEquipped(InInstanceID, CachedCharacter->GetPersistentID());
	//从背包移除
	RootPanelWidget->InventoryPanelWidget->RemoveItemFromSlot(ItemInDB);
	//添加到人物
	RootPanelWidget->InventoryPanelWidget->AddItemToCharacterSlot(ItemInDB);
}


//数据库中动态数据同步给背包
bool UWarInventoryComponent::SyncJsonToBag() const
{
	RootPanelWidget->InventoryPanelWidget->ClearAllSlots();

	UWarGameInstanceSubSystem* Subsystem = UGameplayStatics::GetGameInstance(this)->GetSubsystem<UWarGameInstanceSubSystem>();
	if (!Subsystem) return false;
	UWarPersistentSystem* PersistentSystem = Subsystem->GetWarPersistentSystem();
	if (!PersistentSystem) return false;
	AWarHeroCharacter* Character = Cast<AWarHeroCharacter>(GetOwner());
	if (!Character) return false;

	TArray<FInventoryItemInDB> InventoryInJson;
	//找到当前未装备的
	PersistentSystem->FindInventoryByState(Character->GetPersistentID(), false, InventoryInJson);
	if (InventoryInJson.IsEmpty()) return false;

	for (const FInventoryItemInDB& ItemInDB : InventoryInJson)
	{
		//TODO:同步显示
		RootPanelWidget->InventoryPanelWidget->AddItemToSlot(ItemInDB);
	}
	return true;
}


// 根据当前人物的 Socket 名称，检查是否已存在挂载的装备，返回该装备的 Actor 指针（无则返回 nullptr）
TWeakObjectPtr<AInventoryBase> UWarInventoryComponent::FindSavedInventoryInSlots(const FGuid& InInstanceID) const
{
	if (!InInstanceID.IsValid()) return nullptr;

	const TWeakObjectPtr<AInventoryBase>* FindItem = SavedInventoryInSlots.Find(InInstanceID);

	if (FindItem && FindItem->IsValid())
	{
		AInventoryBase* Inventory = FindItem->Get();

		UWarGameInstanceSubSystem* Subsystem = UGameplayStatics::GetGameInstance(this)->GetSubsystem<UWarGameInstanceSubSystem>();
		if (!Subsystem) return nullptr;
		UWarPersistentSystem* PersistentSystem = Subsystem->GetWarPersistentSystem();
		if (!PersistentSystem) return nullptr;
		FInventoryItemInDB ItemInDB;
		PersistentSystem->FindInventoryByID(InInstanceID, CachedCharacter->GetPersistentID(), ItemInDB);

		const FWarInventoryRow* EquippedRow = UWarGameInstanceSubSystem::FindInventoryRow(this, ItemInDB.TableRowID);
		if (Inventory->GetAttachParentSocketName() == EquippedRow->SocketName)
		{
			// print(TEXT("正常执行了FindSavedInventoryInSlots。"));
			return Inventory;
		}
	}
	return nullptr;
}

//给一个ID返回已经装备的同类型的ID
FGuid UWarInventoryComponent::FindSomeTypeEquippedID(const FGuid& InInstanceID)
{
	UWarGameInstanceSubSystem* Subsystem = UGameplayStatics::GetGameInstance(this)->GetSubsystem<UWarGameInstanceSubSystem>();
	if (!Subsystem) return FGuid();

	UWarPersistentSystem* PersistentSystem = Subsystem->GetWarPersistentSystem();
	if (!PersistentSystem) return FGuid();

	FInventoryItemInDB NewItem;
	if (!PersistentSystem->FindInventoryByID(InInstanceID, CachedCharacter->GetPersistentID(), NewItem) || !NewItem.InstanceID.IsValid())
	{
		return FGuid(); // 原物品无效
	}
	const FWarInventoryRow* NewItemInTable = UWarGameInstanceSubSystem::FindInventoryRow(this, NewItem.TableRowID);
	if (!NewItemInTable) return FGuid();

	for (const auto& Item : SavedInventoryInSlots)
	{
		if (Item.Key == InInstanceID) continue; // 跳过自己

		FInventoryItemInDB CurrentItemInDB;
		if (!PersistentSystem->FindInventoryByID(Item.Key, CachedCharacter->GetPersistentID(), CurrentItemInDB)) continue;

		const FWarInventoryRow* CurrentItemInTable = UWarGameInstanceSubSystem::FindInventoryRow(this, CurrentItemInDB.TableRowID);
		if (!CurrentItemInTable) continue;

		if (CurrentItemInTable->SlotType == NewItemInTable->SlotType)
		{
			return Item.Key;
		}
	}
	return FGuid();
}


void UWarInventoryComponent::DestroyEquippedInventory(const TWeakObjectPtr<AInventoryBase>& Inventory)
{
	if (Inventory.IsValid())
	{
		// 先解除挂载
		Inventory->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		// 销毁世界的实例
		Inventory->Destroy();
	}
}


// 从身上取下放入背包
void UWarInventoryComponent::UnequipInventory(const FGuid& InInstanceID)
{
	if (!InInstanceID.IsValid()) return;

	UWarGameInstanceSubSystem* Subsystem = UGameplayStatics::GetGameInstance(this)->GetSubsystem<UWarGameInstanceSubSystem>();
	if (!Subsystem) return;

	UWarPersistentSystem* PersistentSystem = Subsystem->GetWarPersistentSystem();
	if (!PersistentSystem) return;
	//回退装备
	RollbackSpawnInventory(InInstanceID);
	//修改数据库
	PersistentSystem->MarkAsUnEquipped(InInstanceID, CachedCharacter->GetPersistentID());

	FInventoryItemInDB InventoryInDB;
	PersistentSystem->FindInventoryByID(InInstanceID, CachedCharacter->GetPersistentID(), InventoryInDB);

	//移除人物UI
	RootPanelWidget->InventoryPanelWidget->RemoveItemFromCharacter(InventoryInDB);
	//移除背包UI
	RootPanelWidget->InventoryPanelWidget->AddItemToSlot(InventoryInDB);
}


bool UWarInventoryComponent::CreateWeapon(const UObject* WorldContextObject, const FName& TableID, const FGuid& InPlayerID)
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

		const FInventoryItemInDB& NewWeapon = FInventoryItemInDB::Init(Params);
		UWarGameInstanceSubSystem* Subsystem = WorldContextObject->GetWorld()->GetGameInstance()->GetSubsystem<UWarGameInstanceSubSystem>();
		if (!Subsystem) return false;
		UWarPersistentSystem* PersistentSystem = Subsystem->GetWarPersistentSystem();
		if (!PersistentSystem) return false;
		PersistentSystem->InsertInventoryWithMax(NewWeapon);

		return true;
	}
	return false;
}


bool UWarInventoryComponent::CreateArmor(const UObject* WorldContextObject, const FName& TableID, const FGuid& InPlayerID)
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
		PersistentSystem->InsertInventoryWithMax(NewArmor);
		return true;
	}
	return false;
}

bool UWarInventoryComponent::CreateConsumable(const UObject* WorldContextObject, const FName& TableID, const FGuid& InPlayerID)
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
		PersistentSystem->InsertInventoryWithMax(NewConsumable);

		return true;
	}
	return false;
}

bool UWarInventoryComponent::CreateQuestItem(const UObject* WorldContextObject, const FName& TableID, const FGuid& InPlayerID)
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
		PersistentSystem->InsertInventoryWithMax(NewQuest);
		return true;
	}
	return false;
}

bool UWarInventoryComponent::CreateSkill(const UObject* WorldContextObject, const FName& TableID, const FGuid& InPlayerID)
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
		PersistentSystem->InsertInventoryWithMax(NewSkill);
		return true;
	}
	return false;
}

bool UWarInventoryComponent::GenerateItemToBagAndSaved(const UObject* WorldContextObject, const FName& TableID, const FGuid& PlayerID)
{
	if (TableID.IsNone() || !PlayerID.IsValid()) return false;

	if (!WorldContextObject) return false;
	const FWarInventoryRow* ItemRow = UWarGameInstanceSubSystem::FindInventoryRow(WorldContextObject, TableID);
	if (!ItemRow) return false;

	switch (ItemRow->InventoryType)
	{
	case EWarInventoryType::Armor:
		CreateArmor(WorldContextObject, TableID, PlayerID);
		return true;
	case EWarInventoryType::Consumable:
		CreateConsumable(WorldContextObject, TableID, PlayerID);
		return true;
	case EWarInventoryType::Weapon:
		CreateWeapon(WorldContextObject, TableID, PlayerID);
		return true;
	case EWarInventoryType::Skill:
		CreateSkill(WorldContextObject, TableID, PlayerID);
		return true;
	case EWarInventoryType::QuestItem:
		CreateQuestItem(WorldContextObject, TableID, PlayerID);
		return true;
	case EWarInventoryType::Material:
	case EWarInventoryType::None:
	default:
		return false;
	}
}
