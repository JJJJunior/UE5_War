#include "WarInventoryComponent.h"
#include "Characters/Hero/WarHeroCharacter.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Tools/MyLog.h"
#include "GameInstance/WarGameInstanceSubSystem.h"
#include "War/WorldActors/Inventory/InventoryBase.h"
#include "war/WarComponents/InventorySystem/UI/InventoryPanel/InventoryPanelWidget.h"
#include "war/WarComponents/InventorySystem/UI/CharacterPanel/CharacterPanelWidget.h"
#include "War/WarComponents/InventorySystem/UI/RootPanel/RootPanelWidget.h"
#include "WarComponents/PersistentSystem/WarPersistentSystem.h"


UWarInventoryComponent::UWarInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UWarInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	CachedOwnerCharacter = CastChecked<AWarCharacterBase>(GetOwner());
	if (!CachedOwnerCharacter.IsValid())
	{
		print(TEXT("CachedOwnerCharacter 弱指针无效"));
		return;
	}
	InitRootUI();
	InitInventories();
}

void UWarInventoryComponent::InitRootUI()
{
	checkf(RootPanelWidgetClass, TEXT("RootPanelWidgetClass 没有配置"));
	RootPanelWidget = CreateWidget<URootPanelWidget>(GetWorld(), RootPanelWidgetClass);
	// AddToViewport(主视图)
	RootPanelWidget->AddToViewport();
}

//初始化装备数据主库
void UWarInventoryComponent::InitInventories()
{
	const TArray<FName>& WeaponList = {FName("Katana"), FName("KatanaScabbard"), FName("Katana"), FName("KatanaScabbard")};

	for (int32 i = 0; i < WeaponList.Num(); i++)
	{
		GenerateItemToBagAndSaved(WeaponList[i]);
	}
}


void UWarInventoryComponent::GenerateItemToBagAndSaved(const FName& TableID)
{
	const FWarInventoryRow* ItemRow = UWarGameInstanceSubSystem::FindInventoryRow(this, TableID);

	// 创建物品
	// 2. 准备创建参数
	FCreateInventoryDataParams Params;
	Params.TableRowID = TableID;
	Params.InventoryType = ItemRow->InventoryType;
	Params.InstanceID = FGuid::NewGuid();
	Params.PlayerID = CachedOwnerCharacter->GetPersistentActorID();
	Params.Count = 1; // 默认数量1
	FWeaponData NewWeapon;
	NewWeapon.Init(Params);
	//持久化存储
	UWarPersistentSystem::AddInventoryToDB(this, NewWeapon);
	//构造背包数据
	FItemInBagData BagData = FItemInBagData::CreateInBagData(Params.InstanceID, Params.TableRowID, Params.Count, Params.InventoryType);
	//添加背包
	AddInventory(BagData);
	// UWarPersistentSystem::CheckInventoriesInDB(this);
}


//添加到背包
void UWarInventoryComponent::AddInventory(const FItemInBagData& InBagData)
{
	if (!InBagData.InstanceID.IsValid())
	{
		print(TEXT("AddInventory 无效ID %s"), *InBagData.InstanceID.ToString());
		return;
	}
	Inventories.Add(InBagData);
	//同步UI显示
	RootPanelWidget->InventoryPanelWidget->AddItemToSlot(InBagData);
	// print(TEXT("当前总物品数量 AllInventoryData %d"), Inventories.Num());
}


//打开或者关闭背包
void UWarInventoryComponent::ToggleInventoryUI()
{
	bInventoryUIVisible = !bInventoryUIVisible;
	NotifyUIStateChanged();
	if (bInventoryUIVisible)
	{
		RootPanelWidget->InventoryPanelWidget->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		RootPanelWidget->InventoryPanelWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UWarInventoryComponent::ToggleCharacterUI()
{
	bCharacterUIVisible = !bCharacterUIVisible;
	NotifyUIStateChanged();
	if (bCharacterUIVisible)
	{
		RootPanelWidget->CharacterPanelWidget->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		RootPanelWidget->CharacterPanelWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}


//根据名称生成装备到人物身上(ok)
void UWarInventoryComponent::SpawnInventory(const FItemInBagData& InBagData)
{
	if (!InBagData.InstanceID.IsValid()) return;

	//非装备类型不能生成
	if (InBagData.InventoryType != EWarInventoryType::Armor && InBagData.InventoryType != EWarInventoryType::Weapon) return;

	//当前Socket中存在指针就略过
	if (HasInventoryInSocket(InBagData)) return;

	//异步生成装备加载class的武器类
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = CachedOwnerCharacter.Get();
	SpawnParameters.Instigator = CachedOwnerCharacter.Get();
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	SpawnParameters.TransformScaleMethod = ESpawnActorScaleMethod::MultiplyWithRoot;
	FVector SpawnLocation = FVector::ZeroVector;
	FRotator SpawnRotation = FRotator::ZeroRotator;

	const FWarInventoryRow* ItemRow = UWarGameInstanceSubSystem::FindInventoryRow(this, InBagData.TableRowID);

	if (!ItemRow->InventorySoftClass.IsValid())
	{
		FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
		Streamable.RequestAsyncLoad(ItemRow->InventorySoftClass.ToSoftObjectPath(), [this,SpawnLocation, SpawnRotation,SpawnParameters,ItemRow,InBagData]()
		{
			UClass* LoadedClass = ItemRow->InventorySoftClass.Get();
			if (!LoadedClass)
			{
				print(TEXT("UWarInventoryComponent::SpawnInventory | LoadedClass is nullptr"));
				return;
			}
			TObjectPtr<AInventoryBase> InventoryActor = GetWorld()->SpawnActor<AInventoryBase>(LoadedClass, SpawnLocation, SpawnRotation, SpawnParameters);
			//移入socket
			InventoryActor->AttachToComponent(CachedOwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, ItemRow->SocketName);
			//消除interaction的碰撞体
			InventoryActor->DisableInteractionSphere();
			// 缓存指针
			SavedInventoryInSlots.Emplace(InBagData.InstanceID, InventoryActor);
		});
	}
	else
	{
		UClass* LoadedClass = ItemRow->InventorySoftClass.Get();
		if (!LoadedClass)
		{
			print(TEXT("UWarInventoryComponent::SpawnInventory | LoadedClass is nullptr"));
			return;
		}
		//生成世界物品
		TObjectPtr<AInventoryBase> InventoryActor = GetWorld()->SpawnActor<AInventoryBase>(LoadedClass, SpawnLocation, SpawnRotation, SpawnParameters);
		//移入socket
		InventoryActor->AttachToComponent(CachedOwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, ItemRow->SocketName);
		//消除interaction的碰撞体
		InventoryActor->DisableInteractionSphere();
		// 缓存指针
		SavedInventoryInSlots.Emplace(InBagData.InstanceID, InventoryActor);
	}

	if (SavedInventoryInSlots.Contains(InBagData.InstanceID))
	{
		print(TEXT("SpawnInventory 当前InstanceToActorMap %s 已经存储指针"), *InBagData.InstanceID.ToString());
	}
	//存放装备
	EquippedItems.Add(InBagData);
	//同步UI显示
	RootPanelWidget->CharacterPanelWidget->AddItemToSlot(InBagData);
	print(TEXT("SpawnInventory 了装备 %s"), *InBagData.InstanceID.ToString());
}

//从背包拿装备穿身上
void UWarInventoryComponent::EquipInventory(const FItemInBagData& InBagData)
{
	if (!InBagData.InstanceID.IsValid()) return;

	//当前包里面没有装备就忽略
	if (!Inventories.Contains(InBagData))return;

	//不是装备就略过
	if (InBagData.InventoryType != EWarInventoryType::Armor && InBagData.InventoryType != EWarInventoryType::Weapon) return;

	//防止崩溃
	if (!CachedOwnerCharacter.IsValid() || !CachedOwnerCharacter->GetMesh())
	{
		return;
	}

	TWeakObjectPtr<AInventoryBase> Inventory = FindSavedInventoryInSlots(InBagData);

	if (Inventory.IsValid())
	{
		for (const TPair<FGuid, TWeakObjectPtr<AInventoryBase>>& Pair : SavedInventoryInSlots)
		{
			if (Pair.Value.IsValid() && Pair.Value.Get() == Inventory.Get())
			{
				print(TEXT("InID 有同类型装备已经穿戴 InstanceToActorMap ID：%s"), *Pair.Key.ToString());
				if (TOptional<FItemInBagData> ItemData = FindItemBagDataFromEquipped(Pair.Key))
				{
					UnequipInventory(ItemData.GetValue());
				}
				break; // 如果只允许一件装备同类型
			}
		}
	}

	//生成物品
	SpawnInventory(InBagData);

	//从背包移除
	GetWorld()->GetTimerManager().SetTimerForNextTick([this,InBagData]()
	{
		//背包移除指针
		Inventories.Remove(InBagData);
		//移除背包UI
		RootPanelWidget->InventoryPanelWidget->RemoveItemFromSlot(InBagData);
		//添加人物UI
		RootPanelWidget->CharacterPanelWidget->AddItemToSlot(InBagData);

		ShowCurrentInventories();
	});
}

// 根据当前人物的 Socket 名称，检查是否已存在挂载的装备，返回该装备的 InstanceID（无则返回无效 ID）
bool UWarInventoryComponent::HasInventoryInSocket(const FItemInBagData& InBagData) const
{
	if (!InBagData.InstanceID.IsValid()) return false;

	// 获取当前人物的 Socket 名称
	if (!CachedOwnerCharacter.IsValid() || !CachedOwnerCharacter->GetMesh())
	{
		return false;
	}
	const FName CurrentSocketName = CachedOwnerCharacter->GetMesh()->GetAttachSocketName();
	// 遍历当前已装备的物品
	for (const FItemInBagData& EquippedItem : EquippedItems)
	{
		if (EquippedItem.InstanceID == InBagData.InstanceID) continue;

		const FWarInventoryRow* EquippedRow = UWarGameInstanceSubSystem::FindInventoryRow(this, InBagData.TableRowID);
		if (!EquippedRow) return false;

		// 如果已装备物品的 Socket 和当前人物 Socket 相同，返回它的 InstanceID		
		if (EquippedRow->SocketName == CurrentSocketName)
		{
			return true;
		}
	}
	return false;
}


// 根据当前人物的 Socket 名称，检查是否已存在挂载的装备，返回该装备的 Actor 指针（无则返回 nullptr）
TWeakObjectPtr<AInventoryBase> UWarInventoryComponent::FindSavedInventoryInSlots(const FItemInBagData& InBagData) const
{
	if (!InBagData.InstanceID.IsValid()) return nullptr;

	const FWarInventoryRow* EquippedRow = UWarGameInstanceSubSystem::FindInventoryRow(this, InBagData.TableRowID);

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
void UWarInventoryComponent::UnequipInventory(const FItemInBagData& InBagData)
{
	if (!InBagData.InstanceID.IsValid()) return;

	// if (!EquippedItems.Contains(InBagData)) return;

	if (EquippedItems.Contains(InBagData))
	{
		print(TEXT("包含。。。。。。。。。。。"));
	}
	else
	{
		print(TEXT("%s %d %s"), *InBagData.InstanceID.ToString(), InBagData.InventoryType, *InBagData.TableRowID.ToString());
		print(TEXT("不包含。。。。。。。。。。。"));
		print(TEXT("%d"), EquippedItems.Num());

		for (const auto& Item : EquippedItems)
		{
			print(TEXT("EquippedItems --- %s %d %s"), *Item.InstanceID.ToString(), Item.InventoryType, *InBagData.TableRowID.ToString());
		}

		return;
	}


	// 删除 Character 面板上的装备
	EquippedItems.Remove(InBagData);

	// Character UI 删除装备
	RootPanelWidget->CharacterPanelWidget->RemoveItemFromSlot(InBagData);

	// 添加到背包
	Inventories.Add(InBagData);

	// 背包 UI 添加装备
	RootPanelWidget->InventoryPanelWidget->AddItemToSlot(InBagData);

	// 销毁世界 Socket 上的装备
	TWeakObjectPtr<AInventoryBase> InventoryPtr = FindSavedInventoryInSlots(InBagData);
	if (!InventoryPtr.IsValid())
	{
		print(TEXT("InventoryPtr 无效"));
		return;
	}
	// 先解除挂载
	InventoryPtr->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	// 销毁世界的实例
	InventoryPtr->Destroy();
	SavedInventoryInSlots.Remove(InBagData.InstanceID);
	print(TEXT("UnequipInventory 装备 %s"), *InBagData.InstanceID.ToString());

	ShowCurrentInventories();
}


void UWarInventoryComponent::ShowCurrentInventories() const
{
	print(TEXT("| Inventory Type        | Count |"));
	print(TEXT("|-----------------------|-------|"));
	print(TEXT("| EquippedItems  | %5d |"), EquippedItems.Num());
	print(TEXT("| Inventories  | %5d |"), Inventories.Num());
	// UWarPersistentSystem::CheckInventoriesInDB(this);
}


TOptional<FItemInBagData> UWarInventoryComponent::FindItemBagDataFromEquipped(const FGuid& InID)
{
	for (const auto& Pair : EquippedItems)
	{
		if (Pair.InstanceID == InID)
		{
			return Pair;
		}
	}
	return TOptional<FItemInBagData>();
}
