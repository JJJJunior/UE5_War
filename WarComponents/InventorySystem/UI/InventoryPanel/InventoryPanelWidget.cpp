#include "InventoryPanelWidget.h"
#include "Characters/Hero/WarHeroCharacter.h"
#include "Components/WrapBox.h"
#include "War/WarComponents/InventorySystem/UI/ItemSlotWidget.h"
#include "war/WarComponents/InventorySystem/WarInventoryComponent.h"
#include "Misc/StringBuilder.h"
#include "Tools/MyLog.h"
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
	InitSlots();
	SyncSlots();
}

// 初始化背包槽位
void UInventoryPanelWidget::InitSlots()
{
	if (!InventoryWrapBox) return;
	for (int32 i = 0; i < MaxSlots; i++)
	{
		if (UItemSlotWidget* InventorySlot = CreateWidget<UItemSlotWidget>(GetWorld(), ItemSlotWidgetClass))
		{
			//设置溯源
			InventorySlot->ItemDataInSlot.ParentPanel = "Inventory";
			//设置索引
			InventorySlot->ItemDataInSlot.SlotIndex = i;
			InventorySlot->SetVisibility(ESlateVisibility::Visible);
			InventoryWrapBox->AddChildToWrapBox(InventorySlot);
			CurrentInventorySlots.Add(InventorySlot);
		}
	}
}


// 同步背包数据
void UInventoryPanelWidget::SyncSlots()
{
	ClearAllSlots();
	TArray<FItemInBagData> CurrentInventories = CachedCharacter->GetWarInventoryComponent()->GetCurrentInInventories();
	if (CurrentInventories.Num() > 0)
	{
		for (const FItemInBagData& Inventory : CurrentInventories)
		{
			AddItemToSlot(Inventory);
		}
	}
}


// 清空所有槽位
void UInventoryPanelWidget::ClearAllSlots()
{
	for (auto& InSlot : CurrentInventorySlots)
	{
		if (InSlot)
		{
			InSlot->CleanSlot(); // 这里改成清空整个槽位数据
		}
	}
}

// 添加物品（支持堆叠）
void UInventoryPanelWidget::AddItemToSlot(const FItemInBagData& InBagData)
{
	if (InBagData.InventoryType == EWarInventoryType::None && InBagData.InventoryType == EWarInventoryType::Skill) return;

	// 1. 查找是否有相同类型且未满的槽位
	UItemSlotWidget* SuitableSlot = FindSuitableSlot(InBagData.TableRowID);

	if (SuitableSlot)
	{
		SuitableSlot->AddInventoryToSlot(InBagData);
	}
	else
	{
		// 2. 如果没有，找空槽位
		UItemSlotWidget* EmptySlot = FindFirstEmptySlot();
		if (EmptySlot)
		{
			EmptySlot->AddInventoryToSlot(InBagData);
			EmptySlot->ItemDataInSlot.ParentPanel = "Inventory";
			EmptySlot->ItemDataInSlot.EquipmentSlotType = EEquipmentSlotType::None;
		}
		else
		{
			print(TEXT("背包已满！"));
		}
	}
}


// 从背包移除装备
void UInventoryPanelWidget::RemoveItemFromSlot(const FItemInBagData& InBagData)
{
	for (auto& InSlot : CurrentInventorySlots)
	{
		if (InSlot->ItemDataInSlot.bIsEmpty) continue;
		
		// 判断这个格子的 InstanceIDs 是否包含目标实例 ID
		if (InSlot->ItemDataInSlot.InstanceID == InBagData.InstanceID.ToString())
		{
			// 执行移除
			InSlot->RemoveItemByInstanceID(InBagData);
			break; // 找到立即退出
		}
	}
}

// 查找同类型且未满的格子
UItemSlotWidget* UInventoryPanelWidget::FindSuitableSlot(const FName& TableRowID)
{
	for (UItemSlotWidget* Slot_ : CurrentInventorySlots)
	{
		if (!Slot_->ItemDataInSlot.bIsEmpty && Slot_->ItemDataInSlot.CachedTableRowID == TableRowID)
		{
			if (Slot_->ItemDataInSlot.InventoryInSlots < Slot_->ItemDataInSlot.MaxCount)
			{
				return Slot_;
			}
		}
	}
	return nullptr;
}

// 查找第一个空格子
UItemSlotWidget* UInventoryPanelWidget::FindFirstEmptySlot()
{
	for (UItemSlotWidget* Slot_ : CurrentInventorySlots)
	{
		if (Slot_->ItemDataInSlot.bIsEmpty)
		{
			return Slot_;
		}
	}
	return nullptr;
}
