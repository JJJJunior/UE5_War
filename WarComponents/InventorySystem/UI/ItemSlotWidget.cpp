#include "ItemSlotWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "CharacterPanel/CharacterPanelWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "WarComponents/InventorySystem/StaticData/WarInventoryDataTableRow.h"
#include "War/Characters/Hero/WarHeroCharacter.h"
#include "WarComponents/InventorySystem/WarInventoryComponent.h"
#include "Windows/AllowWindowsPlatformTypes.h"

void UItemSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	CachedCharacter = Cast<AWarHeroCharacter>(GetOwningPlayerPawn());
	check(CachedCharacter);
	Show();
}


// 添加装备
void UItemSlotWidget::AddInventoryToSlot(const FGuid& InID)
{
	const FInventoryInstanceData* FindData = CachedCharacter->GetWarInventoryComponent()->FindInventoryDataByGuid(InID);
	if (!FindData)
	{
		UE_LOG(LogTemp, Error, TEXT("AddInventoryToSlot: 未找到物品数据！"));
		return;
	}

	// 如果格子是空的，初始化
	if (ItemDataInSlot.bIsEmpty)
	{
		ItemDataInSlot.CachedTableRowID = FindData->TableRowID; // 设置物品ID
		SetMaxCount(InID);
	}

	// 判断是否为同类型物品
	if (ItemDataInSlot.CachedTableRowID != FindData->TableRowID)
	{
		UE_LOG(LogTemp, Warning, TEXT("AddInventoryToSlot: 物品类型不一致！"));
		return;
	}

	// 判断格子是否已满
	if (ItemDataInSlot.InventoryInSlots >= ItemDataInSlot.MaxCount)
	{
		UE_LOG(LogTemp, Warning, TEXT("UInventorySlotWidget::AddInventoryToSlot | 格子满了!"));
		return;
	}

	// 执行叠加
	ItemDataInSlot.InstanceIDs.Add(InID);
	ItemDataInSlot.InventoryInSlots++;
	ItemDataInSlot.bIsEmpty = false;

	const FWarInventoryRow* ItemRow = CachedCharacter->GetWarInventoryComponent()->FindItemRowByGuid(InID);
	if (ItemRow)
	{
		UE_LOG(LogTemp, Warning, TEXT("AddInventoryToSlot | %s | 当前：%d | 最大：%d!"), *ItemRow->Name, ItemDataInSlot.InstanceIDs.Num(), ItemDataInSlot.MaxCount);
	}
	Show();
}


//设置堆叠范围
void UItemSlotWidget::SetMaxCount(const FGuid& InID)
{
	const FWarInventoryRow* ItemRow = CachedCharacter->GetWarInventoryComponent()->FindItemRowByGuid(InID);
	if (!ItemRow)
	{
		UE_LOG(LogTemp, Error, TEXT("SetMaxCount: 找不到物品行数据！"));
		ItemDataInSlot.MaxCount = 1;
		return;
	}

	switch (ItemRow->InventoryType)
	{
	case EWarInventoryType::Equipment:
	case EWarInventoryType::Skill:
		ItemDataInSlot.MaxCount = 1;
		break;
	case EWarInventoryType::QuestItem:
	case EWarInventoryType::Consumable:
	case EWarInventoryType::Material:
		ItemDataInSlot.MaxCount = 99;
		break;
	default:
		ItemDataInSlot.MaxCount = 1;
		break;
	}
}

void UItemSlotWidget::RemoveItem()
{
	if (ItemDataInSlot.InventoryInSlots <= 0 || ItemDataInSlot.InstanceIDs.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("格子已空！"));
		return;
	}

	// 从数组中移除最后一个物品实例
	ItemDataInSlot.InstanceIDs.RemoveAt(ItemDataInSlot.InstanceIDs.Num() - 1);

	ItemDataInSlot.InventoryInSlots--;

	// 判断是否为空
	if (ItemDataInSlot.InventoryInSlots == 0)
	{
		ItemDataInSlot.CachedTableRowID = NAME_None; // 清空类型
		ItemDataInSlot.bIsEmpty = true;
	}

	Show();
}

// 移除指定物品实例
void UItemSlotWidget::RemoveItemByInstanceID(const FGuid& InID)
{
	if (ItemDataInSlot.InventoryInSlots <= 0 || ItemDataInSlot.InstanceIDs.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("格子已空！"));
		return;
	}

	if (!ItemDataInSlot.InstanceIDs.Contains(InID))
	{
		UE_LOG(LogTemp, Warning, TEXT("未找到该物品实例ID！"));
		return;
	}

	// 删除指定实例
	ItemDataInSlot.InstanceIDs.Remove(InID);
	ItemDataInSlot.InventoryInSlots--;

	// 如果删除后数量为0，清空物品类型
	if (ItemDataInSlot.InventoryInSlots == 0)
	{
		ItemDataInSlot.CachedTableRowID = NAME_None;
		ItemDataInSlot.bIsEmpty = true;
	}

	Show();
}

// 显示物品
void UItemSlotWidget::Show()
{
	if (ItemDataInSlot.bIsEmpty || ItemDataInSlot.InventoryInSlots == 0)
	{
		CleanSlot();
		return;
	}

	// 获取当前显示的物品实例ID
	if (ItemDataInSlot.InstanceIDs.Num() == 0)
	{
		CleanSlot();
		return;
	}

	FGuid InstanceID = ItemDataInSlot.InstanceIDs[0];

	const FWarInventoryRow* ItemRow = CachedCharacter->GetWarInventoryComponent()->FindItemRowByGuid(InstanceID);
	if (!ItemRow)
	{
		CleanSlot();
		return;
	}

	// 缓存图片路径
	ItemDataInSlot.CachedTexture = ItemRow->Texture.LoadSynchronous();

	if (!ItemDataInSlot.CachedTexture)
	{
		CleanSlot();
		return;
	}

	ItemImage->SetBrushFromTexture(ItemDataInSlot.CachedTexture);

	// 显示数量（1个时隐藏数量文本）
	if (ItemDataInSlot.InventoryInSlots > 1)
	{
		ItemQuantity->SetText(FText::AsNumber(ItemDataInSlot.InventoryInSlots));
	}
	else
	{
		ItemQuantity->SetText(FText::GetEmpty());
	}
}


void UItemSlotWidget::CleanSlot()const
{
	ItemQuantity->SetText(FText::FromString(TEXT("")));
	ItemImage->SetBrushFromTexture(nullptr);
}


//拿到到列表中的第一个实例ID
FGuid UItemSlotWidget::GetFirstInstanceID(const FName& TableRowID) const
{
	if (ItemDataInSlot.CachedTableRowID == TableRowID && ItemDataInSlot.InventoryInSlots > 0)
	{
		return ItemDataInSlot.InstanceIDs[0];
	}
	return FGuid();
}


//开始拖拽（NativeOnDragDetected）
void UItemSlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	// 创建拖拽事件
	UDragDropOperation* TempDragOperation = UWidgetBlueprintLibrary::CreateDragDropOperation(UDragDropOperation::StaticClass());
	if (TempDragOperation)
	{
		// 设置拖拽携带的内容（可以传递 ItemID）
		TempDragOperation->Payload = this;
		TempDragOperation->DefaultDragVisual = this;
		TempDragOperation->Pivot = EDragPivot::MouseDown;
		TempDragOperation->Offset = FVector2D::ZeroVector;

		// 创建拖拽过程中的图标（通常建议做一个拖拽物品样式的蓝图 Widget）
		OutOperation = TempDragOperation;
	}
}

bool UItemSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UItemSlotWidget* SourceSlot = Cast<UItemSlotWidget>(InOperation->Payload);
	if (!SourceSlot || SourceSlot == this)
	{
		return false;
	}
	//有东西格子才能拖动
	if (SourceSlot->ItemDataInSlot.InventoryInSlots > 0)
	{
		const FGuid& InstanceID = SourceSlot->GetFirstInstanceID(SourceSlot->ItemDataInSlot.CachedTableRowID);
		const FWarInventoryRow* ItemRow = CachedCharacter->GetWarInventoryComponent()->FindItemRowByGuid(InstanceID);
		
		// 拖入的是角色装备面板
		if (ItemDataInSlot.ParentPanel == "Character" && SourceSlot->ItemDataInSlot.ParentPanel == "Inventory")
		{
			const EEquipmentSlotType& SourceEquipmentSlotType = ItemRow->EquipmentSlotType;
			// 判断两者的装备的位置相同
			if (SourceEquipmentSlotType == ItemDataInSlot.EquipmentSlotType)
			{
				CachedCharacter->GetWarInventoryComponent()->EquipInventory(InstanceID);
				return true;
			}
		}
		// 从角色面板到装备面板
		if (ItemDataInSlot.ParentPanel == "Inventory" && SourceSlot->ItemDataInSlot.ParentPanel == "Character")
		{
			// 卸下装备
			CachedCharacter->GetWarInventoryComponent()->UnequipInventory(InstanceID);
			return true;
		}
		return false;
	}
	return false;
}

FReply UItemSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FGuid InstanceID = GetFirstInstanceID(ItemDataInSlot.CachedTableRowID);
	//有装备的情况下
	if (ItemDataInSlot.InventoryInSlots > 0)
	{
		//鼠标右键在人物面板和装备面板切换装备
		if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
		{
			if (ItemDataInSlot.ParentPanel == "Inventory")
			{
				CachedCharacter->GetWarInventoryComponent()->EquipInventory(InstanceID);
			}
			else if (ItemDataInSlot.ParentPanel == "Character")
			{
				CachedCharacter->GetWarInventoryComponent()->UnequipInventory(InstanceID);
			}
			// 可以在这里做：显示菜单、装备物品、使用物品等
			return FReply::Handled();
		}

		if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
		{
			return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
		}
	}
	return FReply::Unhandled();
}


void UItemSlotWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FGuid InstanceID = GetFirstInstanceID(ItemDataInSlot.CachedTableRowID);
	if (ItemDataInSlot.InventoryInSlots > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s %s %d"), *InstanceID.ToString(), *ItemDataInSlot.ParentPanel, ItemDataInSlot.EquipmentSlotType);
	}
}
