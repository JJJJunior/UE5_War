﻿#include "ItemSlotWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GameInstance/WarGameInstanceSubSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Tools/MyLog.h"
#include "WarComponents/InventorySystem/StaticData/WarInventoryDataTableRow.h"
#include "War/Characters/Hero/WarHeroCharacter.h"
#include "WarComponents/InventorySystem/WarInventoryComponent.h"
#include "WarComponents/PersistentSystem/WarPersistentSystem.h"
#include "Windows/AllowWindowsPlatformTypes.h"

void UItemSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	CachedCharacter = Cast<AWarHeroCharacter>(GetOwningPlayerPawn());
	if (!CachedCharacter.IsValid())return;

	UWarGameInstanceSubSystem* SubSystem = UGameplayStatics::GetGameInstance(this)->GetSubsystem<UWarGameInstanceSubSystem>();
	if (!SubSystem)return;
	CachedPersistentSystem = SubSystem->GetWarPersistentSystem();
	if (!CachedPersistentSystem.IsValid())return;
}


void UItemSlotWidget::InitSlot(const ESlotType& InSlotType)
{
	// print(TEXT("UItemSlotWidget::InitSlot"));
	CleanSlot();
	SlotData.SlotType = InSlotType;
	bIsInitialized = true;
	Show();
}


// 添加装备
bool UItemSlotWidget::AddToSlot(const FInventoryItemInDB& ItemInDB)
{
	// print(TEXT("UItemSlotWidget::AddToSlot"));
	if (!bIsInitialized) return false;

	//检查InstanceID TableRowID InventoryType
	if (!CachedPersistentSystem->HasInventory(ItemInDB.InstanceID, CachedCharacter->GetPersistentID())) return false;

	if (SlotData.bIsFull) return false;

	SetMaxCount(ItemInDB);

	if ((SlotData.Count + ItemInDB.Count) > SlotData.MaxCount)
	{
		// print(TEXT("SlotData Count is full"));
		return false;
	}
	const FWarInventoryRow* ItemRow = UWarGameInstanceSubSystem::FindInventoryRow(this, ItemInDB.TableRowID);
	if (!ItemRow) return false;

	SlotData.InstanceID = ItemInDB.InstanceID;
	SlotData.TableRowID = ItemInDB.TableRowID;
	SlotData.Texture = ItemRow->Texture.LoadSynchronous();
	SlotData.Count += ItemInDB.Count;
	SlotData.bIsFull = SlotData.Count == SlotData.MaxCount;

	Show();
	return true;
}

//设置堆叠范围
void UItemSlotWidget::SetMaxCount(const FInventoryItemInDB& ItemInDB)
{
	if (!bIsInitialized) return;

	switch (ItemInDB.InventoryType)
	{
	case EWarInventoryType::Weapon:
	case EWarInventoryType::Armor:
	case EWarInventoryType::Skill:
		SlotData.MaxCount = 1;
		break;
	default:
		SlotData.MaxCount = 99;
		break;
	}
}

// 显示物品
void UItemSlotWidget::Show() const
{
	// print(TEXT("UItemSlotWidget::Show"));

	if (SlotData.Count <= 0) return;

	if (SlotData.Count > 1)
	{
		ItemQuantity->SetText(FText::AsNumber(SlotData.Count));
	}
	else
	{
		ItemQuantity->SetText(FText::GetEmpty());
	}

	ItemImage->SetBrushFromTexture(SlotData.Texture);
}


// 移除指定物品实例
bool UItemSlotWidget::RemoveFromSlot(const FInventoryItemInDB& ItemInDB)
{
	if (!bIsInitialized) return false;
	// print(TEXT("UItemSlotWidget::RemoveFromSlot"));

	//检查InstanceID TableRowID InventoryType
	if (!CachedPersistentSystem->HasInventory(ItemInDB.InstanceID, CachedCharacter->GetPersistentID())) return false;

	if (SlotData.TableRowID != ItemInDB.TableRowID) return false;

	//数量为0
	if (SlotData.Count <= 0) return false;

	const FWarInventoryRow* ItemRow = UWarGameInstanceSubSystem::FindInventoryRow(this, ItemInDB.TableRowID);
	if (!ItemRow) return false;

	if ((SlotData.Count - ItemInDB.Count) < 0)
	{
		print(TEXT("移除数量结果小于0，跳过"));
		return false;
	}

	SlotData.Count -= ItemInDB.Count;
	// 如果删除后数量为0，清空物品数据
	if (SlotData.Count == 0)
	{
		CleanSlot();
	}
	Show();
	return true;
}


void UItemSlotWidget::CleanSlot()
{
	ItemQuantity->SetText(FText::GetEmpty());
	ItemImage->SetBrushFromTexture(nullptr);
	SlotData.bIsFull = false;
	SlotData.Count = 0;
	SlotData.MaxCount = 0;
	SlotData.InstanceID = FGuid();
	SlotData.TableRowID = FName();
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
	if (SourceSlot->SlotData.Count > 0)
	{
		return true;
	}
	return false;
}

FReply UItemSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	//有装备的情况下
	if (SlotData.Count > 0)
	{
		//鼠标右键在人物面板和装备面板切换装备
		if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
		{
			//判断类型物品类型
			const FWarInventoryRow* FindRow = UWarGameInstanceSubSystem::FindInventoryRow(this, SlotData.TableRowID);
			switch (FindRow->InventoryType)
			{
			case EWarInventoryType::Weapon:
			case EWarInventoryType::Armor:
				// print(TEXT("右键了%s"), *SlotData.TableRowID.ToString());
				//背包里面的物品右键逻辑
				if (SlotData.SlotType == ESlotType::Normal)
				{
					if (UWarInventoryComponent* InventoryComponent = CachedCharacter->GetWarInventoryComponent())
					{
						FGuid FindID = InventoryComponent->FindSomeTypeEquippedID(SlotData.InstanceID);
						if (FindID.IsValid())
						{
							InventoryComponent->UnequipInventory(FindID);
						}
						InventoryComponent->EquipInventory(SlotData.InstanceID);
					}
				}
				//角色身上右键逻辑
				if (SlotData.SlotType != ESlotType::Normal && SlotData.SlotType != ESlotType::None && SlotData.SlotType != ESlotType::Skill)
				{
					if (UWarInventoryComponent* InventoryComponent = CachedCharacter->GetWarInventoryComponent())
					{
						InventoryComponent->UnequipInventory(SlotData.InstanceID);
					}
				}
				break;
			default:
				print(TEXT("右键处理其他物品类型的逻辑"));
				break;
			}
			return FReply::Handled();
		}

		if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
		{
			print(TEXT("左键拖动了%s"), *SlotData.TableRowID.ToString());
			return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
		}
	}
	return FReply::Unhandled();
}


void UItemSlotWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (SlotData.Count > 0)
	{
		// print(TEXT("%s %d %s"), *SlotData.InstanceID.ToString(), SlotData.Count, *SlotData.TableRowID.ToString());
	}
}
