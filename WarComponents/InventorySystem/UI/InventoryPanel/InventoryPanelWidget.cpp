#include "InventoryPanelWidget.h"
#include "Characters/Hero/WarHeroCharacter.h"
#include "Components/WrapBox.h"
#include "War/WarComponents/InventorySystem/UI/ItemSlotWidget.h"
#include "war/WarComponents/InventorySystem/WarInventoryComponent.h"
#include "Misc/StringBuilder.h"

void UInventoryPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	CachedCharacter = Cast<AWarHeroCharacter>(GetOwningPlayerPawn());
	check(CachedCharacter);
	InitSlots();
	SyncSlots();
}

// 初始化背包槽位
void UInventoryPanelWidget::InitSlots()
{
	checkf(ItemSlotWidgetClass, TEXT("ItemSlotWidgetClass 没有配置"));

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

	if (AWarHeroCharacter* Character = Cast<AWarHeroCharacter>(GetOwningPlayerPawn()))
	{
		for (const auto& Inventory : Character->GetWarInventoryComponent()->GetCurrentInInventories())
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
void UInventoryPanelWidget::AddItemToSlot(const FGuid& InID)
{
	// 获取物品信息
	const FInventoryInstanceData* FindData = CachedCharacter->GetWarInventoryComponent()->FindInventoryDataByGuid(InID);
	if (!FindData)
	{
		UE_LOG(LogTemp, Error, TEXT("AddItem: 未找到物品数据！"));
		return;
	}

	// 1. 查找是否有相同类型且未满的槽位
	UItemSlotWidget* SuitableSlot = FindSuitableSlot(FindData->TableRowID);

	if (SuitableSlot)
	{
		SuitableSlot->AddInventoryToSlot(InID);
	}
	else
	{
		// 2. 如果没有，找空槽位
		UItemSlotWidget* EmptySlot = FindFirstEmptySlot();
		if (EmptySlot)
		{
			EmptySlot->AddInventoryToSlot(InID);
			EmptySlot->ItemDataInSlot.ParentPanel = "Inventory";
			EmptySlot->ItemDataInSlot.EquipmentSlotType = EEquipmentSlotType::None;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("背包已满！"));
		}
	}
}


// 从背包移除装备
void UInventoryPanelWidget::RemoveItemFromSlot(const FGuid& InID)
{
	for (auto& InSlot : CurrentInventorySlots)
	{
		if (InSlot->ItemDataInSlot.bIsEmpty) continue;

		// 判断这个格子的 InstanceIDs 是否包含目标实例 ID
		if (InSlot->ItemDataInSlot.InstanceIDs.Contains(InID))
		{
			// 执行移除
			InSlot->RemoveItemByInstanceID(InID);
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
