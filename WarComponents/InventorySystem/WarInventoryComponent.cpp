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

	//当前Socket中存在指针就略过
	if (HasInventoryInSocket(InventoryItemDB)) return;

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
			InventoryActor->DisableInteractionSphere();
			//移入socket
			if (InventoryActor->AttachToComponent(Character->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, ItemRow->SocketName))
			{
				// 缓存指针
				SavedInventoryInSlots.Emplace(InInstanceID, InventoryActor);
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
		if (!InventoryActor)
		{
			print_err(TEXT("装备InventoryActor 指针丢失"));
			return;
		}
		//消除interaction的碰撞体
		InventoryActor->DisableInteractionSphere();
		//移入socket
		if (InventoryActor->AttachToComponent(Character->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, ItemRow->SocketName))
		{
			// 缓存指针
			SavedInventoryInSlots.Emplace(InInstanceID, InventoryActor);
		}
	}
	//TODO:将数据标记为已装备
	if (PersistentSystem->MarkAsEquipped(InInstanceID, CachedCharacter->GetPersistentID()))
	{
		//TODO:同步显示背包装备 以及 人物面板装备
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

	AWarHeroCharacter* Character = Cast<AWarHeroCharacter>(GetOuter());
	if (!Character) return;
	//不是装备就略过
	if (ItemInDB.InventoryType != EWarInventoryType::Armor && ItemInDB.InventoryType != EWarInventoryType::Weapon) return;

	//防止崩溃
	if (!Character || !Character->GetMesh())
	{
		return;
	}
	TWeakObjectPtr<AInventoryBase> Inventory = FindSavedInventoryInSlots(ItemInDB);
	if (Inventory.IsValid())
	{
		for (const TPair<FGuid, TWeakObjectPtr<AInventoryBase>>& Pair : SavedInventoryInSlots)
		{
			if (Pair.Value.IsValid() && Pair.Value.Get() == Inventory.Get())
			{
				print(TEXT("InID 有同类型装备已经穿戴 InstanceToActorMap ID：%s"), *Pair.Key.ToString());
				if (PersistentSystem->HasEquipped(Pair.Key, CachedCharacter->GetPersistentID()))
				{
					UnequipInventory(Pair.Key);
				}
				break; // 如果只允许一件装备同类型
			}
		}
	}
	//生成物品
	SpawnInventory(InInstanceID);
	//TODO:从背包移除
	RootPanelWidget->InventoryPanelWidget->RemoveItemFromSlot(ItemInDB);
	//TODO:添加到人物
	//显示debug信息
}

// 根据当前人物的 Socket 名称，检查是否已存在挂载的装备
bool UWarInventoryComponent::HasInventoryInSocket(const FInventoryItemInDB& ItemInDB) const
{
	if (!ItemInDB.InstanceID.IsValid() || !CachedCharacter.IsValid() || !CachedCharacter->GetMesh())
	{
		return false;
	}

	const FName CurrentSocketName = CachedCharacter->GetMesh()->GetAttachSocketName();

	UWarGameInstanceSubSystem* Subsystem = UGameplayStatics::GetGameInstance(this)->GetSubsystem<UWarGameInstanceSubSystem>();
	if (!Subsystem) return false;

	UWarPersistentSystem* PersistentSystem = Subsystem->GetWarPersistentSystem();
	if (!PersistentSystem) return false;

	// 遍历当前已装备的物品
	TArray<FInventoryItemInDB> ItemInDBArray;
	PersistentSystem->FindEquippedInventory(ItemInDB.InstanceID, CachedCharacter->GetPersistentID(), ItemInDBArray);
	for (const FInventoryItemInDB& EquippedItem : ItemInDBArray)
	{
		// 排除自身（比如重新装备同一个物品）
		if (EquippedItem.InstanceID == ItemInDB.InstanceID) continue;

		const FWarInventoryRow* EquippedRow = UWarGameInstanceSubSystem::FindInventoryRow(this, EquippedItem.TableRowID);
		if (!EquippedRow) return false;

		// 如果已装备物品的 Socket 和当前人物 Socket 相同，返回它的 InstanceID		
		if (EquippedRow->SocketName == CurrentSocketName)
		{
			return true;
		}
	}
	return false;
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
	PersistentSystem->FindAllInventoriesByPlayerID(Character->GetPersistentID(), InventoryInJson);
	if (InventoryInJson.IsEmpty()) return false;

	for (const FInventoryItemInDB& ItemInDB : InventoryInJson)
	{
		//TODO:同步显示
		RootPanelWidget->InventoryPanelWidget->AddItemToSlot(ItemInDB);
	}
	return true;
}


// 根据当前人物的 Socket 名称，检查是否已存在挂载的装备，返回该装备的 Actor 指针（无则返回 nullptr）
TWeakObjectPtr<AInventoryBase> UWarInventoryComponent::FindSavedInventoryInSlots(const FInventoryItemInDB& ItemInDB) const
{
	if (!ItemInDB.InstanceID.IsValid()) return nullptr;

	const FWarInventoryRow* EquippedRow = UWarGameInstanceSubSystem::FindInventoryRow(this, ItemInDB.TableRowID);

	for (const auto& Pair : SavedInventoryInSlots)
	{
		if (!Pair.Value.IsValid()) continue;

		const USceneComponent* Parent = Pair.Value->GetAttachParentActor() ? Pair.Value->GetAttachParentActor()->GetRootComponent() : nullptr;
		if (!Parent) continue;

		if (Pair.Value->GetAttachParentSocketName() == EquippedRow->SocketName)
		{
			return Pair.Value.Get();
		}
	}
	return nullptr;
}


// 从身上取下放入背包
void UWarInventoryComponent::UnequipInventory(const FGuid& InInstanceID)
{
	if (!InInstanceID.IsValid()) return;

	UWarGameInstanceSubSystem* Subsystem = UGameplayStatics::GetGameInstance(this)->GetSubsystem<UWarGameInstanceSubSystem>();
	if (!Subsystem) return;

	UWarPersistentSystem* PersistentSystem = Subsystem->GetWarPersistentSystem();
	if (!PersistentSystem) return;

	// TODO:删除 Character 面板上的装备
	// RootPanelWidget->CharacterPanelWidget->RemoveItemFromSlot(InBagData);

	// 修改标记
	if (PersistentSystem->MarkAsUnEquipped(InInstanceID, CachedCharacter->GetPersistentID()))
	{
		FInventoryItemInDB ItemInDB;
		PersistentSystem->FindInventoryByID(InInstanceID, CachedCharacter->GetPersistentID(), ItemInDB);
		// 背包 UI 添加装备
		RootPanelWidget->InventoryPanelWidget->AddItemToSlot(ItemInDB);

		// 销毁世界 Socket 上的装备
		TWeakObjectPtr<AInventoryBase> InventoryPtr = FindSavedInventoryInSlots(ItemInDB);
		if (InventoryPtr.IsValid())
		{
			// 先解除挂载
			InventoryPtr->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			// 销毁世界的实例
			InventoryPtr->Destroy();
			SavedInventoryInSlots.Remove(InInstanceID);
			print(TEXT("UnequipInventory 装备 %s"), *InInstanceID.ToString());
		}
	}
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
		PersistentSystem->InsertInventory(NewWeapon);

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
		PersistentSystem->InsertInventory(NewArmor);
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
		PersistentSystem->InsertInventory(NewConsumable);

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
		PersistentSystem->InsertInventory(NewQuest);
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
		PersistentSystem->InsertInventory(NewSkill);
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
		CreateWeapon(WorldContextObject, TableID, PlayerID);
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
