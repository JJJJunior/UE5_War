#include "WarInventoryComponent.h"
#include "Characters/Hero/WarHeroCharacter.h"
#include "DataManager/WarDataManager.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Tools/MyLog.h"
#include "GameInstance/WarGameInstanceSubSystem.h"
#include "War/WorldActors/Inventory/InventoryBase.h"
#include "war/WarComponents/InventorySystem/UI/InventoryPanel/InventoryPanelWidget.h"
#include "war/WarComponents/InventorySystem/UI/CharacterPanel/CharacterPanelWidget.h"
#include "War/WarComponents/InventorySystem/UI/RootPanel/RootPanelWidget.h"


UWarInventoryComponent::UWarInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UWarInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	InitRootUI();
}

void UWarInventoryComponent::InitRootUI()
{
	checkf(RootPanelWidgetClass, TEXT("RootPanelWidgetClass 没有配置"));
	RootPanelWidget = CreateWidget<URootPanelWidget>(GetWorld(), RootPanelWidgetClass);
	// AddToViewport(主视图)
	RootPanelWidget->AddToViewport();
}


bool UWarInventoryComponent::GenerateItemToBagAndSaved(const FName& TableID)
{
	bool bSuccess = false;

	if (!TableID.IsValid())
	{
		print(TEXT("TableID %s"), *TableID.ToString())
		return false;
	}

	AWarHeroCharacter* Character = Cast<AWarHeroCharacter>(GetOwner());
	if (!Character) return false;

	const FWarInventoryRow* FindItemRow = UWarGameInstanceSubSystem::FindInventoryRow(this, TableID);
	if (!FindItemRow) return false;

	switch (FindItemRow->InventoryType)
	{
	case EWarInventoryType::Weapon:
		{
			FItemInBagData Weapon;
			UWarDataManager::CreateWeapon(this, TableID, Character->GetPersistentID(), Weapon);
			if (Weapon.InstanceID.IsValid())
			{
				//添加背包
				AddInventory(Weapon);
				bSuccess = true;
			}
		}
		break;
	case EWarInventoryType::Armor:
		{
			FItemInBagData Armor;
			UWarDataManager::CreateArmor(this, TableID, Character->GetPersistentID(), Armor);
			if (Armor.InstanceID.IsValid())
			{
				AddInventory(Armor);
				bSuccess = true;
			}
		}
		break;
	case EWarInventoryType::Consumable:
		{
			FItemInBagData Consumable;
			UWarDataManager::CreateConsumable(this, TableID, Character->GetPersistentID(), Consumable);
			if (Consumable.InstanceID.IsValid())
			{
				AddInventory(Consumable);
				bSuccess = true;
			}
		}
		break;
	case EWarInventoryType::QuestItem:
		{
			FItemInBagData QuestItem;
			UWarDataManager::CreateQuestItem(this, TableID, Character->GetPersistentID(), QuestItem);
			if (QuestItem.InstanceID.IsValid())
			{
				AddInventory(QuestItem);
				bSuccess = true;
			}
		}
		break;
	case EWarInventoryType::Skill:
		{
			FItemInBagData Skill;
			UWarDataManager::CreateSkill(this, TableID, Character->GetPersistentID(), Skill);
			if (Skill.InstanceID.IsValid())
			{
				AddInventory(Skill);
				bSuccess = true;
			}
		}
		break;
	case EWarInventoryType::Material:
	case EWarInventoryType::None:
	default:
		bSuccess = false;
		break;
	}

	return bSuccess;
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
	// print(TEXT("添加了新物品 %s"), *InBagData.InstanceID.ToString());
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

	if (ItemRow->InventorySoftClass.IsValid())
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
			if (!IsValid(InventoryActor))
			{
				print_err(TEXT("装备InventoryActor 指针丢失"));
				return;
			}
			//消除interaction的碰撞体
			InventoryActor->DisableInteractionSphere();
			//移入socket
			if (InventoryActor->AttachToComponent(CachedOwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, ItemRow->SocketName))
			{
				// 缓存指针
				SavedInventoryInSlots.Emplace(InBagData.InstanceID, InventoryActor);
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
		if (InventoryActor->AttachToComponent(CachedOwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, ItemRow->SocketName))
		{
			//消除interaction的碰撞体
			InventoryActor->DisableInteractionSphere();
			// 缓存指针
			SavedInventoryInSlots.Emplace(InBagData.InstanceID, InventoryActor);
		}
	}
}

//从背包拿装备穿身上
void UWarInventoryComponent::EquipInventory(const FItemInBagData& InBagData)
{
	if (InBagData.InstanceID.IsValid() && Inventories.Contains(InBagData))
	{
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
		Inventories.Remove(InBagData);
		//添加到人物
		EquippedItems.Add(InBagData);
		//移除背包UI
		RootPanelWidget->InventoryPanelWidget->RemoveItemFromSlot(InBagData);
		//同步UI显示
		RootPanelWidget->CharacterPanelWidget->AddItemToSlot(InBagData);
		print(TEXT("EquipInventory 了装备 %s"), *InBagData.InstanceID.ToString());
		ShowCurrentInventories();
	}
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
	if (InBagData.InstanceID.IsValid() && EquippedItems.Contains(InBagData))
	{
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
		if (InventoryPtr.IsValid())
		{
			// 先解除挂载
			InventoryPtr->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			// 销毁世界的实例
			InventoryPtr->Destroy();
			SavedInventoryInSlots.Remove(InBagData.InstanceID);
			print(TEXT("UnequipInventory 装备 %s"), *InBagData.InstanceID.ToString());
		}
		ShowCurrentInventories();
	}
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
