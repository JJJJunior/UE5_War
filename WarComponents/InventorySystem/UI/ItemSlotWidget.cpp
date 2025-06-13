#include "ItemSlotWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "CharacterPanel/CharacterPanelWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GameInstance/WarGameInstanceSubSystem.h"
#include "InventoryPanel/InventoryPanelWidget.h"
#include "WarComponents/InventorySystem/StaticData/WarInventoryDataTableRow.h"
#include "War/Characters/Hero/WarHeroCharacter.h"
#include "WarComponents/InventorySystem/WarInventoryComponent.h"
#include "Windows/AllowWindowsPlatformTypes.h"

void UItemSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	Show();
}


TObjectPtr<UDataTable> UItemSlotWidget::GetInventoryDataTable() const
{
	UWarGameInstanceSubSystem* Subsystem = GetWorld()->GetGameInstance()->GetSubsystem<UWarGameInstanceSubSystem>();
	if (!IsValid(Subsystem))
	{
		UE_LOG(LogTemp, Error, TEXT("WarSubsystem or its dependencies are invalid."));
		return nullptr;
	}
	return Subsystem->GetCachedWarInventoryDataTable();
}

// 添加装备
void UItemSlotWidget::AddInventoryToSlot(const FGuid& InID)
{
	if (!ItemDataInSlot.bIsEmpty)
	{
		// UE_LOG(LogTemp, Warning, TEXT("UInventorySlotWidget::AddInventoryToSlot | 满了!"));
		return;
	}
	
	//保存
	ItemDataInSlot.CachedInstanceID = InID;

	if (ItemDataInSlot.MaxCount == 0)
	{
		SetMaxCount();
	}

	if (ItemDataInSlot.InventoryInSlots >= ItemDataInSlot.MaxCount)
	{
		// UE_LOG(LogTemp, Warning, TEXT("UInventorySlotWidget::AddInventoryToSlot | 格子满了!"));
		return;
	}

	ItemDataInSlot.InventoryInSlots++;

	ItemDataInSlot.bIsEmpty = ItemDataInSlot.InventoryInSlots == 0;

	// UE_LOG(LogTemp, Warning, TEXT("添加了一个物品 %s 数量 %d 格子状态 %d"), *CachedInstanceID.ToString(), InventoryInSlots, bIsEmpty);
	Show();
}


//设置堆叠范围
void UItemSlotWidget::SetMaxCount()
{
	AWarHeroCharacter* Character = Cast<AWarHeroCharacter>(GetOwningPlayerPawn());
	if (!Character) return;

	const FInventoryInstanceData* InstanceData = Character->GetWarInventoryComponent()->FindInventoryDataByGuid(ItemDataInSlot.CachedInstanceID);
	if (!InstanceData) return;

	FWarInventoryRow* ItemRow = GetInventoryDataTable()->FindRow<FWarInventoryRow>(InstanceData->TableRowID, "FInventoryRow");
	if (!ItemRow) return;

	switch (ItemRow->InventoryType)
	{
	case EWarInventoryType::Equipment:
		ItemDataInSlot.MaxCount = 1;
		break;
	case EWarInventoryType::Skill:
		ItemDataInSlot.MaxCount = 1;
		break;
	case EWarInventoryType::QuestItem:
		ItemDataInSlot.MaxCount = 99;
		break;
	case EWarInventoryType::Consumable:
		ItemDataInSlot.MaxCount = 99;
		break;
	case EWarInventoryType::Material:
		ItemDataInSlot.MaxCount = 99;
		break;
	default:
		break;
	}
}

// 移除装备
void UItemSlotWidget::RemoveItem()
{
	if (ItemDataInSlot.InventoryInSlots > 0)
	{
		ItemDataInSlot.InventoryInSlots--;
		ItemDataInSlot.bIsEmpty = ItemDataInSlot.InventoryInSlots == 0;
	}
	else
	{
		ItemDataInSlot.InventoryInSlots = 0;
		ItemDataInSlot.bIsEmpty = true;
	}

	if (ItemDataInSlot.bIsEmpty)
	{
		if (ItemQuantity)
		{
			ItemQuantity->SetText(FText::FromString(TEXT("")));
		}

		if (ItemImage)
		{
			ItemImage->SetBrushFromTexture(nullptr);
		}
	}
	else
	{
		Show();
	}
}


// 显示物品
void UItemSlotWidget::Show()
{
	if (ItemDataInSlot.bIsEmpty || ItemDataInSlot.InventoryInSlots == 0) return;

	AWarHeroCharacter* Character = Cast<AWarHeroCharacter>(GetOwningPlayerPawn());
	if (!Character)
	{
		CleanSlot();
		return;
	}
	const FInventoryInstanceData* InstanceData = Character->GetWarInventoryComponent()->FindInventoryDataByGuid(ItemDataInSlot.CachedInstanceID);
	if (!InstanceData)
	{
		CleanSlot();
		return;
	};

	FWarInventoryRow* ItemRow = GetInventoryDataTable()->FindRow<FWarInventoryRow>(InstanceData->TableRowID, "FInventoryRow");
	if (!ItemRow)
	{
		CleanSlot();
		return;
	};
	//缓存图片路径
	ItemDataInSlot.CachedTexture = ItemRow->Texture.LoadSynchronous();

	if (!ItemDataInSlot.CachedTexture)
	{
		CleanSlot();
		return;
	}
	ItemImage->SetBrushFromTexture(ItemDataInSlot.CachedTexture);
	if (ItemDataInSlot.InventoryInSlots > 1)
	{
		ItemQuantity->SetText(FText::AsNumber(ItemDataInSlot.InventoryInSlots));
	}
	// UE_LOG(LogTemp, Warning, TEXT("当前显示: %s"), *LoadedTexture.GetPath())
}

void UItemSlotWidget::CleanSlot()
{
	ItemQuantity->SetText(FText::FromString(TEXT("")));
	ItemImage->SetBrushFromTexture(nullptr);
	ItemDataInSlot = FItemDataInSlot();
}

EEquipmentSlotType UItemSlotWidget::GetSlotTypeByInstanceID(const FGuid& InID) const
{
	AWarHeroCharacter* Character = Cast<AWarHeroCharacter>(GetOwningPlayerPawn());
	// 获取来源数据
	const FInventoryInstanceData* SourceInstanceData = Character->GetWarInventoryComponent()->FindInventoryDataByGuid(InID);
	if (!SourceInstanceData)
	{
		UE_LOG(LogTemp, Warning, TEXT("SourceInstanceData is null!"));
		return EEquipmentSlotType();
	}
	FWarInventoryRow* SourceRow = GetInventoryDataTable()->FindRow<FWarInventoryRow>(SourceInstanceData->TableRowID, "FInventoryRow");
	if (!SourceRow)
	{
		UE_LOG(LogTemp, Warning, TEXT("SourceRow is null!"));
		return EEquipmentSlotType();
	}
	return SourceRow->EquipmentSlotType;
}


FReply UItemSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	//有装备的情况下
	if (ItemDataInSlot.InventoryInSlots > 0)
	{
		AWarHeroCharacter* Character = Cast<AWarHeroCharacter>(GetOwningPlayerPawn());

		//鼠标右键在人物面板和装备面板切换装备
		if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
		{
			if (ItemDataInSlot.ParentPanel == "Inventory")
			{
				Character->GetWarInventoryComponent()->EquipInventory(ItemDataInSlot.CachedInstanceID);
			}
			else if (ItemDataInSlot.ParentPanel == "Character")
			{
				Character->GetWarInventoryComponent()->UnequipInventory(ItemDataInSlot.CachedInstanceID);
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

	AWarHeroCharacter* Character = Cast<AWarHeroCharacter>(GetOwningPlayerPawn());
	if (!Character)
	{
		return false;
	}

	//有东西格子才能拖动
	if (SourceSlot->ItemDataInSlot.InventoryInSlots > 0)
	{
		// 拖入的是角色装备面板
		if (ItemDataInSlot.ParentPanel == "Character" && SourceSlot->ItemDataInSlot.ParentPanel == "Inventory")
		{
			const EEquipmentSlotType& SourceEquipmentSlotType = GetSlotTypeByInstanceID(SourceSlot->ItemDataInSlot.CachedInstanceID);
			// 判断两者的装备的位置相同
			if (SourceEquipmentSlotType == ItemDataInSlot.EquipmentSlotType)
			{
				Character->GetWarInventoryComponent()->EquipInventory(SourceSlot->ItemDataInSlot.CachedInstanceID);
				return true;
			}
		}
		// 从角色面板到装备面板
		if (ItemDataInSlot.ParentPanel == "Inventory" && SourceSlot->ItemDataInSlot.ParentPanel == "Character")
		{
			// 卸下装备
			Character->GetWarInventoryComponent()->UnequipInventory(SourceSlot->ItemDataInSlot.CachedInstanceID);
			return true;
		}
		return false;
	}
	return false;
}

void UItemSlotWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (ItemDataInSlot.InventoryInSlots > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s %s %d"), *ItemDataInSlot.CachedInstanceID.ToString(), *ItemDataInSlot.ParentPanel, ItemDataInSlot.EquipmentSlotType);
	}
}
