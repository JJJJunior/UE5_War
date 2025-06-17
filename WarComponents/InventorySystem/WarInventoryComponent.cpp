#include "WarInventoryComponent.h"
#include "Characters/Hero/WarHeroCharacter.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "GameInstance/WarGameInstanceSubSystem.h"
#include "War/WorldActors/Inventory/InventoryBase.h"
#include "war/WarComponents/InventorySystem/UI/InventoryPanel/InventoryPanelWidget.h"
#include "war/WarComponents/InventorySystem/UI/CharacterPanel/CharacterPanelWidget.h"
#include "War/WarComponents/InventorySystem/UI/RootPanel/RootPanelWidget.h"
#include "War/WarComponents/InventorySystem/DynamicData/InventoryInstanceData.h"


UWarInventoryComponent::UWarInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


TObjectPtr<UDataTable> UWarInventoryComponent::GetInventoryDataTable() const
{
	UWarGameInstanceSubSystem* Subsystem = GetWorld()->GetGameInstance()->GetSubsystem<UWarGameInstanceSubSystem>();
	if (!IsValid(Subsystem))
	{
		UE_LOG(LogTemp, Error, TEXT("WarSubsystem or its dependencies are invalid."));
		return nullptr;
	}
	return Subsystem->GetCachedWarInventoryDataTable();
}


void UWarInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	CachedOwnerCharacter = CastChecked<AWarCharacterBase>(GetOwner());
	check(CachedOwnerCharacter);

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
	const TArray<FName> WeaponList = {FName("Katana"), FName("KatanaScabbard"), FName("Katana"), FName("KatanaScabbard")};

	for (int32 i = 0; i < WeaponList.Num(); i++)
	{
		FInventoryInstanceData NewItem = GenerateNewWeapon(WeaponList[i]);
		AddInventory(NewItem);
	}
}

//添加到背包
void UWarInventoryComponent::AddInventory(const FInventoryInstanceData& NewData)
{
	AllInventoryData.Add(NewData.InstanceID, NewData);
	CurrentInInventories.Add(NewData.InstanceID);
	//同步UI显示
	RootPanelWidget->InventoryPanelWidget->AddItemToSlot(NewData.InstanceID);
}

//查找数据
const FInventoryInstanceData* UWarInventoryComponent::FindInventoryDataByGuid(const FGuid& Guid) const
{
	if (const FInventoryInstanceData* FindData = AllInventoryData.Find(Guid))
	{
		return FindData;
	}
	return nullptr;
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


// TObjectPtr<AInventoryBase> UWarInventoryComponent::GetSceneActor(const FGuid& InstanceID) const
// {
// 	if (!InstanceID.IsValid()) return nullptr;
//
// 	// 1. 检查是否存在于Map
// 	const TObjectPtr<AInventoryBase>* FoundPtr = InstanceToActorMap.Find(InstanceID);
// 	if (!FoundPtr) // 键不存在
// 	{
// 		UE_LOG(LogTemp, Warning, TEXT("GetSceneActor: InstanceID %s 不存在于Map中"), *InstanceID.ToString());
// 		return nullptr;
// 	}
//
// 	// 2. 检查针有效性
// 	if (!IsValid(*FoundPtr)) // 指针存在但对象已销毁
// 	{
// 		UE_LOG(LogTemp, Warning, TEXT("GetSceneActor: InstanceID %s 的Actor已被销毁"), *InstanceID.ToString());
// 		return nullptr;
// 	}
// 	return *FoundPtr;
// }

//根据名称生成装备到人物身上(ok)
void UWarInventoryComponent::SpawnInventory(const FGuid& InID)
{
	if (!InID.IsValid()) return;

	const FInventoryInstanceData* FindData = FindInventoryDataByGuid(InID);
	if (!FindData->InstanceID.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("FindData 不存在"));
		return;
	}

	FWarInventoryRow* ItemRow = GetInventoryDataTable()->FindRow<FWarInventoryRow>(FindData->TableRowID, "Find ItemName");
	if (!ItemRow)
	{
		UE_LOG(LogTemp, Error, TEXT("FWarInventoryRow 不存在"));
		return;
	}

	//非装备类型不能生成
	if (ItemRow->InventoryType != EWarInventoryType::Equipment) return;

	//当前Socket中存在指针就略过
	if (HasInventoryInSocket(InID)) return;

	//异步生成装备加载class的武器类
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = CachedOwnerCharacter;
	SpawnParameters.Instigator = CachedOwnerCharacter;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	SpawnParameters.TransformScaleMethod = ESpawnActorScaleMethod::MultiplyWithRoot;
	FVector SpawnLocation = FVector::ZeroVector;
	FRotator SpawnRotation = FRotator::ZeroRotator;

	if (!ItemRow->InventorySoftClass.IsValid())
	{
		FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
		Streamable.RequestAsyncLoad(ItemRow->InventorySoftClass.ToSoftObjectPath(), [this,SpawnLocation, SpawnRotation,SpawnParameters,ItemRow,InID]()
		{
			UClass* LoadedClass = ItemRow->InventorySoftClass.Get();
			if (!LoadedClass)
			{
				UE_LOG(LogTemp, Error, TEXT("UWarInventoryComponent::SpawnInventory | LoadedClass is nullptr"));
				return;
			}
			TObjectPtr<AInventoryBase> InventoryActor = GetWorld()->SpawnActor<AInventoryBase>(LoadedClass, SpawnLocation, SpawnRotation, SpawnParameters);
			//移入socket
			InventoryActor->AttachToComponent(CachedOwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, ItemRow->SocketName);
			//消除interaction的碰撞体
			InventoryActor->DisableInteractionSphere();
			// 缓存指针
			InstanceToActorMap.Emplace(InID, InventoryActor);
		});
	}
	else
	{
		UClass* LoadedClass = ItemRow->InventorySoftClass.Get();
		if (!LoadedClass)
		{
			UE_LOG(LogTemp, Error, TEXT("UWarInventoryComponent::SpawnInventory | LoadedClass is nullptr"));
			return;
		}
		//生成世界物品
		TObjectPtr<AInventoryBase> InventoryActor = GetWorld()->SpawnActor<AInventoryBase>(LoadedClass, SpawnLocation, SpawnRotation, SpawnParameters);
		//移入socket
		InventoryActor->AttachToComponent(CachedOwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, ItemRow->SocketName);
		//消除interaction的碰撞体
		InventoryActor->DisableInteractionSphere();
		// 缓存指针
		InstanceToActorMap.Emplace(InID, InventoryActor);
	}

	if (InstanceToActorMap.Contains(InID))
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnInventory 当前InstanceToActorMap %s 已经存储指针"), *InID.ToString());
	}
	//存放装备
	CurrentEquippedItems.Add(InID);
	//同步UI显示
	RootPanelWidget->CharacterPanelWidget->AddItemToSlot(InID);

	UE_LOG(LogTemp, Warning, TEXT("SpawnInventory 了装备 %s"), *InID.ToString());
}

//从背包拿装备穿身上
void UWarInventoryComponent::EquipInventory(const FGuid& InID)
{
	if (!InID.IsValid()) return;

	//当前包里面没有装备就忽略
	if (!CurrentInInventories.Contains(InID)) return;
	//防止崩溃
	if (!IsValid(CachedOwnerCharacter) || !CachedOwnerCharacter->GetMesh())
	{
		return;
	}

	if (TObjectPtr<AInventoryBase> Inventory = FindActorInActorMap(InID))
	{
		for (const auto& Pair : InstanceToActorMap)
		{
			if (Pair.Value && Pair.Value == Inventory)
			{
				UE_LOG(LogTemp, Warning, TEXT("InID 有同类型装备已经穿戴 InstanceToActorMap ID：%s"), *Pair.Key.ToString());
				UnequipInventory(Pair.Key);
			}
		}
	}
	//生成物品
	SpawnInventory(InID);

	//从背包移除
	GetWorld()->GetTimerManager().SetTimerForNextTick([this,InID]()
	{
		//背包移除指针
		CurrentInInventories.Remove(InID);
		//移除背包UI
		RootPanelWidget->InventoryPanelWidget->RemoveItemFromSlot(InID);
		//添加人物UI
		RootPanelWidget->CharacterPanelWidget->AddItemToSlot(InID);

		ShowCurrentInventories();
	});
}


// 根据当前人物的 Socket 名称，检查是否已存在挂载的装备，返回该装备的 InstanceID（无则返回无效 ID）
bool UWarInventoryComponent::HasInventoryInSocket(const FGuid& InID) const
{
	if (!InID.IsValid()) return false;

	// 获取当前人物的 Socket 名称
	if (!CachedOwnerCharacter || !CachedOwnerCharacter->GetMesh())
	{
		return false;
	}
	const FName CurrentSocketName = CachedOwnerCharacter->GetMesh()->GetAttachSocketName();
	// 遍历当前已装备的物品
	for (const FGuid& EquippedID : CurrentEquippedItems)
	{
		if (EquippedID == InID) continue;

		const FInventoryInstanceData* EquippedData = FindInventoryDataByGuid(EquippedID);
		if (!EquippedData) continue;

		const FWarInventoryRow* EquippedRow = GetInventoryDataTable()->FindRow<FWarInventoryRow>(EquippedData->TableRowID, "Find Equipped Item");
		if (!EquippedRow) continue;

		// 如果已装备物品的 Socket 和当前人物 Socket 相同，返回它的 InstanceID
		if (EquippedRow->SocketName == CurrentSocketName)
		{
			return true;
		}
	}
	return false;
}


// 根据当前人物的 Socket 名称，检查是否已存在挂载的装备，返回该装备的 Actor 指针（无则返回 nullptr）
TObjectPtr<AInventoryBase> UWarInventoryComponent::FindActorInActorMap(const FGuid& InID) const
{
	if (!InID.IsValid()) return nullptr;

	const FInventoryInstanceData* EquippedData = FindInventoryDataByGuid(InID);
	if (!EquippedData)
	{
		UE_LOG(LogTemp, Warning, TEXT("EquippedData 不存在"));
		return nullptr;
	}

	const FWarInventoryRow* EquippedRow = GetInventoryDataTable()->FindRow<FWarInventoryRow>(EquippedData->TableRowID, "Find Equipped Item");
	if (!EquippedRow)
	{
		UE_LOG(LogTemp, Warning, TEXT("EquippedRow <UNK>"));
		return nullptr;
	}

	for (const auto& Pair : InstanceToActorMap)
	{
		if (!Pair.Value) continue;

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
void UWarInventoryComponent::UnequipInventory(const FGuid& InID)
{
	if (!InID.IsValid()) return;

	if (!CurrentEquippedItems.Contains(InID)) return;

	const FInventoryInstanceData* EquippedData = FindInventoryDataByGuid(InID);
	if (!EquippedData)
	{
		UE_LOG(LogTemp, Warning, TEXT("EquippedData 不存在"));
		return;
	}

	const FWarInventoryRow* EquippedRow = GetInventoryDataTable()->FindRow<FWarInventoryRow>(EquippedData->TableRowID, "Find Equipped Item");
	ensureMsgf(EquippedRow, TEXT("FWarInventoryRow is null"));
	if (!EquippedRow) return;

	// 删除 Character 面板上的装备
	CurrentEquippedItems.Remove(InID);

	// Character UI 删除装备
	RootPanelWidget->CharacterPanelWidget->RemoveItemFromSlot(InID);

	// 添加到背包
	CurrentInInventories.Add(InID);

	// 背包 UI 添加装备
	RootPanelWidget->InventoryPanelWidget->AddItemToSlot(InID);

	// 销毁世界 Socket 上的装备
	TObjectPtr<AInventoryBase> InventoryPtr = FindActorInActorMap(InID);
	if (!InventoryPtr)
	{
		UE_LOG(LogTemp, Error, TEXT("InventoryPtr 无效"));
		return;
	}
	// 先解除挂载
	InventoryPtr->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	// 销毁世界的实例
	InventoryPtr->Destroy();
	InstanceToActorMap.Remove(InID);
	UE_LOG(LogTemp, Warning, TEXT("UnequipInventory 装备 %s"), *InID.ToString());

	ShowCurrentInventories();
}

//调用工厂方法创建武器
FInventoryInstanceData UWarInventoryComponent::GenerateNewWeapon(const FName& InInventoryName)
{
	FInventoryInstanceData NewWeaponData;
	NewWeaponData.InstanceID = FGuid::NewGuid();
	NewWeaponData.TableRowID = InInventoryName;
	NewWeaponData.InventoryType = EWarInventoryType::Equipment;
	NewWeaponData.ExtraData = UWeaponInstanceData::CreateWeaponInstance(this, 100.f, 10.f);
	return NewWeaponData;
}


void UWarInventoryComponent::ShowCurrentInventories() const
{
	UE_LOG(LogTemp, Warning, TEXT("| Inventory Type        | Count |"));
	UE_LOG(LogTemp, Warning, TEXT("|-----------------------|-------|"));
	UE_LOG(LogTemp, Warning, TEXT("| CurrentEquippedItems  | %5d |"), CurrentEquippedItems.Num());
	UE_LOG(LogTemp, Warning, TEXT("| CurrentInInventories  | %5d |"), CurrentInInventories.Num());
	UE_LOG(LogTemp, Warning, TEXT("| AllInventoryData      | %5d |"), AllInventoryData.Num());
}
