#include "ItemSlotWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "CharacterPanel/CharacterPanelWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GameInstance/WarGameInstanceSubSystem.h"
#include "Tools/MyLog.h"
#include "WarComponents/InventorySystem/StaticData/WarInventoryDataTableRow.h"
#include "War/Characters/Hero/WarHeroCharacter.h"
#include "WarComponents/InventorySystem/WarInventoryComponent.h"
#include "Windows/AllowWindowsPlatformTypes.h"

void UItemSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	CachedCharacter = Cast<AWarHeroCharacter>(GetOwningPlayerPawn());
	if (!CachedCharacter.IsValid())
	{
		print(TEXT("CachedCharacter 弱指针无效"));
		return;
	}
	Show();
}


// 添加装备
void UItemSlotWidget::AddInventoryToSlot(const FItemInBagData& InBagData)
{
	// 如果格子是空的，初始化
	if (ItemDataInSlot.bIsEmpty)
	{
		ItemDataInSlot.CachedTableRowID = InBagData.TableRowID; // 设置物品ID
		ItemDataInSlot.ItemInBagData = InBagData; //后续要调用
		ItemDataInSlot.InstanceID = InBagData.InstanceID.ToString();
		SetMaxCount(InBagData);
	}

	//类型不一致跳出
	if (!CheckSameItemType(InBagData)) return;

	// 判断格子是否已满
	if (ItemDataInSlot.InventoryInSlots >= ItemDataInSlot.MaxCount)
	{
		// print(TEXT("UInventorySlotWidget | 格子满了!"));
		return;
	}

	// 执行叠加
	ItemDataInSlot.InventoryInSlots++;
	ItemDataInSlot.bIsEmpty = ItemDataInSlot.InventoryInSlots <= 0;

	//print(TEXT("AddInventoryToSlot | %s | 当前：%d | 最大：%d!"), *ItemDataInSlot.CachedTableRowID.ToString(), ItemDataInSlot.InventoryInSlots, ItemDataInSlot.MaxCount);
	Show();
}


//设置堆叠范围
void UItemSlotWidget::SetMaxCount(const FItemInBagData& InBagData)
{
	const FWarInventoryRow* ItemRow = UWarGameInstanceSubSystem::FindInventoryRow(this, InBagData.TableRowID);

	if (!ItemRow)
	{
		print(TEXT("SetMaxCount: 找不到物品行数据！"));
		ItemDataInSlot.MaxCount = 1;
		return;
	}

	switch (InBagData.InventoryType)
	{
	case EWarInventoryType::Armor:
	case EWarInventoryType::Weapon:
	case EWarInventoryType::Skill:
		ItemDataInSlot.MaxCount = 1;
		break;
	case EWarInventoryType::QuestItem:
	case EWarInventoryType::Consumable:
	case EWarInventoryType::Material:
		ItemDataInSlot.MaxCount = 99;
		break;
	case EWarInventoryType::None:
	default:
		ItemDataInSlot.MaxCount = 1;
		break;
	}
}

// 移除指定物品实例
void UItemSlotWidget::RemoveItemByInstanceID(const FItemInBagData& InBagData)
{
	if (ItemDataInSlot.InstanceID != InBagData.InstanceID.ToString())
	{
		return;
	}

	if (ItemDataInSlot.InventoryInSlots <= 0)
	{
		// print(TEXT("格子已空！"));
		return;
	}

	//类型不一致跳出
	if (!CheckSameItemType(InBagData)) return;

	// 删除指定实例
	ItemDataInSlot.InventoryInSlots--;

	// 如果删除后数量为0，清空物品类型
	if (ItemDataInSlot.InventoryInSlots == 0)
	{
		ItemDataInSlot.CachedTableRowID = NAME_None;
		ItemDataInSlot.bIsEmpty = true;
		ItemDataInSlot.InstanceID = FString();
	}

	Show();
}

// 显示物品
void UItemSlotWidget::Show()
{
	if (ItemDataInSlot.InventoryInSlots == 0)
	{
		CleanSlot();
		return;
	}

	const FWarInventoryRow* ItemRow = UWarGameInstanceSubSystem::FindInventoryRow(this, ItemDataInSlot.CachedTableRowID);
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


void UItemSlotWidget::CleanSlot() const
{
	ItemQuantity->SetText(FText::FromString(TEXT("")));
	ItemImage->SetBrushFromTexture(nullptr);
}

bool UItemSlotWidget::CheckSameItemType(const FItemInBagData& InBagData) const
{
	// 如果槽位是空的，直接返回 true（因为可以放入任何新物品）
	if (ItemDataInSlot.bIsEmpty)
	{
		return true;
	}

	switch (InBagData.InventoryType)
	{
	case EWarInventoryType::Armor:
	case EWarInventoryType::Weapon:
	case EWarInventoryType::Skill:
		if (ItemDataInSlot.InstanceID == InBagData.InstanceID.ToString())
		{
			return true;
		}
	case EWarInventoryType::QuestItem:
	case EWarInventoryType::Consumable:
	case EWarInventoryType::Material:
		if (ItemDataInSlot.CachedTableRowID == InBagData.TableRowID.ToString())
		{
			return true;
		}
	case EWarInventoryType::None:
	default:
		return false;
	}
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
		const FWarInventoryRow* ItemRow = UWarGameInstanceSubSystem::FindInventoryRow(this, SourceSlot->ItemDataInSlot.CachedTableRowID);
		// 拖入的是角色装备面板
		if (ItemDataInSlot.ParentPanel == "Character" && SourceSlot->ItemDataInSlot.ParentPanel == "Inventory")
		{
			const EEquipmentSlotType& SourceEquipmentSlotType = ItemRow->EquipmentSlotType;
			// 判断两者的装备的位置相同
			if (SourceEquipmentSlotType == ItemDataInSlot.EquipmentSlotType)
			{
				//传递要传原来格子数据，不是新格子数据。SourceSlot
				CachedCharacter->GetWarInventoryComponent()->EquipInventory(SourceSlot->ItemDataInSlot.ItemInBagData);
				return true;
			}
		}
		// 从角色面板到装备面板
		if (ItemDataInSlot.ParentPanel == "Inventory" && SourceSlot->ItemDataInSlot.ParentPanel == "Character")
		{
			// 卸下装备
			CachedCharacter->GetWarInventoryComponent()->UnequipInventory(SourceSlot->ItemDataInSlot.ItemInBagData);
			return true;
		}
		return false;
	}
	return false;
}

FReply UItemSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	//有装备的情况下
	if (ItemDataInSlot.InventoryInSlots > 0)
	{
		//鼠标右键在人物面板和装备面板切换装备
		if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
		{
			if (ItemDataInSlot.ParentPanel == "Inventory")
			{
				// print(TEXT("点到Inventory面板了。 %s"), *ItemDataInSlot.InstanceID);
				CachedCharacter->GetWarInventoryComponent()->EquipInventory(ItemDataInSlot.ItemInBagData);
			}
			else if (ItemDataInSlot.ParentPanel == "Character")
			{
				// print(TEXT("点到Character面板了。"));
				CachedCharacter->GetWarInventoryComponent()->UnequipInventory(ItemDataInSlot.ItemInBagData);
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
	if (ItemDataInSlot.InventoryInSlots > 0)
	{
		// print(TEXT("%s %s %d"), *ItemDataInSlot.InstanceID, *ItemDataInSlot.ParentPanel, ItemDataInSlot.EquipmentSlotType);
		// print(TEXT("ItemDataInSlot.ItemInBagData.InstanceID %s"), *ItemDataInSlot.ItemInBagData.InstanceID.ToString());
	}
}
