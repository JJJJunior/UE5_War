#include "InventoryPanelWidget.h"
#include "Characters/Hero/WarHeroCharacter.h"
#include "Components/WrapBox.h"
#include "War/WarComponents/InventorySystem/UI/ItemSlotWidget.h"
#include "war/WarComponents/InventorySystem/WarInventoryComponent.h"
#include "Misc/StringBuilder.h"

void UInventoryPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
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
			InSlot->RemoveItem();
		}
	}
}

// 添加装备到空槽
void UInventoryPanelWidget::AddItemToSlot(const FGuid& InID)
{
	for (auto& InSlot : CurrentInventorySlots)
	{
		// UE_LOG(LogTemp, Warning, TEXT("%s %s"), *InSlot->GetName(), *LexToString(InSlot->bIsEmpty));
		if (InSlot->ItemDataInSlot.bIsEmpty)
		{
			InSlot->AddInventoryToSlot(InID);
			InSlot->ItemDataInSlot.ParentPanel = "Inventory";
			break; // 找到一个空槽就够了
		}
	}
}


// 从背包移除装备
void UInventoryPanelWidget::RemoveItemFromSlot(const FGuid& InID)
{
	for (auto& InSlot : CurrentInventorySlots)
	{
		//如果slot是有东西的,并且格子里存在的ID和传入的ID一致
		if (!InSlot->ItemDataInSlot.bIsEmpty && InSlot->ItemDataInSlot.CachedInstanceID == InID)
		{
			InSlot->RemoveItem();
			break; // 一般背包物品不会重复，可以移除后立即退出
		}
	}
}
