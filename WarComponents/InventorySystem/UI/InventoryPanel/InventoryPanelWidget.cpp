#include "InventoryPanelWidget.h"
#include "Characters/Hero/WarHeroCharacter.h"
#include "Components/WrapBox.h"
#include "War/WarComponents/InventorySystem/UI/ItemSlotWidget.h"
#include "Misc/StringBuilder.h"
#include "Tools/MyLog.h"
#include "Components/SizeBox.h"
#include "DataManager/ConfigData/GameConfigData.h"
#include "GameInstance/WarGameInstanceSubSystem.h"
#include "Kismet/GameplayStatics.h"
#include "WarComponents/PersistentSystem/WarPersistentSystem.h"

void UInventoryPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	checkf(ItemSlotWidgetClass, TEXT("ItemSlotWidgetClass 没有配置"));
	CachedCharacter = Cast<AWarHeroCharacter>(GetOwningPlayerPawn());
	if (!CachedCharacter.IsValid())
	{
		print(TEXT("CachedCharacter 弱指针无效"));
		return;
	}
	MaxSlots = UWarGameInstanceSubSystem::GetGameConfigData(this)->MaxSlots;
	InitSlots();
}

// 初始化背包槽位
void UInventoryPanelWidget::InitSlots()
{
	TMap<ESlotType, USizeBox*> SlotBoxMap = {
		{ESlotType::Head, HeadBox},
		{ESlotType::Body, BodyBox},
		{ESlotType::LeftHand, LeftHandBox},
		{ESlotType::RightHand, RightHandBox},
		{ESlotType::Legs, LegBox},
		{ESlotType::Footer, FooterBox},
		{ESlotType::Neck, NeckBox},
		{ESlotType::Ring1, RingBox1},
		{ESlotType::Ring2, RingBox2},
	};

	int32 index = 0;

	for (const auto& Pair : SlotBoxMap)
	{
		ESlotType SlotType = Pair.Key;
		USizeBox* SlotBox = Pair.Value;

		if (!SlotBox) continue;

		if (UItemSlotWidget* CharacterSlotWidget = CreateWidget<UItemSlotWidget>(GetWorld(), ItemSlotWidgetClass))
		{
			//设置索引
			CharacterSlotWidget->SetVisibility(ESlateVisibility::Visible);
			CharacterSlotWidget->InitSlot(SlotType);
			SlotBox->AddChild(CharacterSlotWidget);
			CharacterSlots.Emplace(SlotType, CharacterSlotWidget);
		}
		index++;
	}

	if (!InventoryWrapBox) return;
	for (int32 i = 0; i < MaxSlots; i++)
	{
		if (UItemSlotWidget* InventorySlotWidget = CreateWidget<UItemSlotWidget>(GetWorld(), ItemSlotWidgetClass))
		{
			//设置索引
			InventorySlotWidget->SetVisibility(ESlateVisibility::Visible);
			InventorySlotWidget->InitSlot(ESlotType::Normal);
			InventoryWrapBox->AddChildToWrapBox(InventorySlotWidget);
			InventorySlots.Add(InventorySlotWidget);
		}
	}
}


// 同步背包数据
bool UInventoryPanelWidget::SyncSlots()
{
	UWarGameInstanceSubSystem* Subsystem = UGameplayStatics::GetGameInstance(this)->GetSubsystem<UWarGameInstanceSubSystem>();
	if (!Subsystem) return false;
	UWarPersistentSystem* PersistentSystem = Subsystem->GetWarPersistentSystem();
	if (!PersistentSystem) return false;

	bool bSuccess = false;

	TArray<FInventoryItemInDB> ItemInDBArray;
	PersistentSystem->FindAllInventoriesByPlayerID(CachedCharacter->GetPersistentID(), ItemInDBArray);

	if (ItemInDBArray.Num() <= 0) return false;

	for (const FInventoryItemInDB& Inventory : ItemInDBArray)
	{
		if (AddItemToSlot(Inventory))
		{
			bSuccess = true;
		}
	}
	return bSuccess;
}

bool UInventoryPanelWidget::AddItemToCharacterSlot(const FInventoryItemInDB& InItemInDB)
{
	const FWarInventoryRow* FindRow = UWarGameInstanceSubSystem::FindInventoryRow(this, InItemInDB.TableRowID);
	if (!FindRow) return false;

	TObjectPtr<UItemSlotWidget>* SlotPtr = CharacterSlots.Find(FindRow->SlotType);
	if (SlotPtr && *SlotPtr && (*SlotPtr)->bIsInitialized)
	{
		return (*SlotPtr)->AddToSlot(InItemInDB);
	}
	return false;
}

bool UInventoryPanelWidget::RemoveItemFromCharacter(const FInventoryItemInDB& InItemInDB)
{
	const FWarInventoryRow* FindRow = UWarGameInstanceSubSystem::FindInventoryRow(this, InItemInDB.TableRowID);
	if (!FindRow) return false;

	TObjectPtr<UItemSlotWidget>* SlotPtr = CharacterSlots.Find(FindRow->SlotType);
	if (SlotPtr && *SlotPtr)
	{
		return (*SlotPtr)->RemoveFromSlot(InItemInDB);
	}
	return false;
}


// 清空所有槽位
void UInventoryPanelWidget::ClearAllSlots()
{
	for (const TObjectPtr<UItemSlotWidget>& InSlot : InventorySlots)
	{
		if (InSlot)
		{
			InSlot->CleanSlot(); // 这里改成清空整个槽位数据
		}
	}
}

// 添加物品（支持堆叠）
bool UInventoryPanelWidget::AddItemToSlot(const FInventoryItemInDB& InItemInDB)
{
	bool bSuccess = false;
	if (UItemSlotWidget* FindSlot = FindSuitableSlot(InItemInDB))
	{
		if (FindSlot->AddToSlot(InItemInDB))
		{
			bSuccess = true;
		}
	}
	return bSuccess;
}


// 查找同类型且未满的格子
UItemSlotWidget* UInventoryPanelWidget::FindSuitableSlot(const FInventoryItemInDB& InItemInDB)
{
	const bool bCanStack = InItemInDB.InventoryType == EWarInventoryType::Consumable ||
		InItemInDB.InventoryType == EWarInventoryType::QuestItem ||
		InItemInDB.InventoryType == EWarInventoryType::Material;

	if (bCanStack)
	{
		// 1. 查找已有未满的堆叠格子
		for (const TObjectPtr<UItemSlotWidget>& InSlot : InventorySlots)
		{
			if (!InSlot->SlotData.bIsFull && InSlot->bIsInitialized && InSlot->SlotData.TableRowID == InItemInDB.TableRowID)
			{
				return InSlot;
			}
		}

		// 2. 没有堆叠目标？找空格子
		for (const TObjectPtr<UItemSlotWidget>& InSlot : InventorySlots)
		{
			if (!InSlot->SlotData.bIsFull && InSlot->bIsInitialized && InSlot->SlotData.TableRowID.IsNone())
			{
				return InSlot;
			}
		}
	}
	else
	{
		// 不可堆叠：找空格子
		for (const TObjectPtr<UItemSlotWidget>& InSlot : InventorySlots)
		{
			if (!InSlot->SlotData.bIsFull &&
				InSlot->bIsInitialized &&
				InSlot->SlotData.TableRowID.IsNone())
			{
				return InSlot;
			}
		}
	}
	return nullptr;
}


// 从背包移除装备
bool UInventoryPanelWidget::RemoveItemFromSlot(const FInventoryItemInDB& InItemInDB)
{
	bool bSuccess = false;

	for (const TObjectPtr<UItemSlotWidget>& InSlot : InventorySlots)
	{
		if (InSlot->SlotData.Count == 0) continue;

		// 判断这个格子的 InstanceIDs 是否包含目标实例 ID
		if (InSlot->SlotData.InstanceID == InItemInDB.InstanceID && InItemInDB.Count > 0)
		{
			// 执行移除
			if (InSlot->RemoveFromSlot(InItemInDB))
			{
				bSuccess = true;
			}
			break; // 找到立即退出
		}
	}

	return bSuccess;
}
