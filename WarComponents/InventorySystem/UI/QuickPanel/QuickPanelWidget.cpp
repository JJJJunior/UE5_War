#include "QuickPanelWidget.h"
#include "WarComponents/InventorySystem/UI/ItemSlotWidget.h"
#include "War/Characters/Hero/WarHeroCharacter.h"
#include "War/WarComponents/InventorySystem/WarInventoryComponent.h"
#include "Components/SizeBox.h"
#include "Components/HorizontalBox.h"


class UWarGameInstanceSubSystem;

void UQuickPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	InitSlots();
	SyncSlots();
}


void UQuickPanelWidget::InitSlots()
{
	checkf(ItemSlotWidgetClass, TEXT("ItemSlotWidgetClass 没有配置"));

	if (!QuickSizeBox) return;

	// 先确保可见
	QuickSizeBox->SetVisibility(ESlateVisibility::Visible);
	this->SetVisibility(ESlateVisibility::Visible);

	for (int32 i = 0; i < MaxSlots; i++)
	{
		if (UItemSlotWidget* QuickSlot = CreateWidget<UItemSlotWidget>(GetWorld(), ItemSlotWidgetClass))
		{
			//设置溯源
			QuickSlot->ItemDataInSlot.ParentPanel = "Quick";
			//设置索引
			QuickSlot->ItemDataInSlot.SlotIndex = i;
			QuickSlot->SetVisibility(ESlateVisibility::Visible);
			QuickWrapBox->AddChildToHorizontalBox(QuickSlot);
			QuickSlots.Add(QuickSlot);
		}
	}
}

void UQuickPanelWidget::SyncSlots()
{
	ClearAllSlots();

	if (AWarHeroCharacter* Character = Cast<AWarHeroCharacter>(GetOwningPlayerPawn()))
	{
		for (const auto& InQuickItem : Character->GetWarInventoryComponent()->GetCurrentInQuickItems())
		{
			AddItemToSlot(InQuickItem);
		}
	}
}

void UQuickPanelWidget::RemoveItemFromSlot(const FGuid& InID)
{
	for (auto& InSlot : QuickSlots)
	{
		//如果slot是有东西的,并且格子里存在的ID和传入的ID一致
		if (!InSlot->ItemDataInSlot.bIsEmpty && InSlot->ItemDataInSlot.CachedInstanceID == InID)
		{
			InSlot->RemoveItem();
			break; // 一般背包物品不会重复，可以移除后立即退出
		}
	}
}

void UQuickPanelWidget::ClearAllSlots()
{
	for (auto& InSlot : QuickSlots)
	{
		if (InSlot)
		{
			InSlot->RemoveItem();
		}
	}
}

//检查类型添加
void UQuickPanelWidget::AddItemToSlot(const FGuid& InID)
{
	AWarHeroCharacter* Character = Cast<AWarHeroCharacter>(GetOwningPlayerPawn());
	ensureMsgf(Character, TEXT("AWarHeroCharacter 不存在"));
	const FInventoryInstanceData* InData = Character->GetWarInventoryComponent()->FindInventoryDataByGuid(InID);
	
	for (auto& InSlot : QuickSlots)
	{
		if (InData->InventoryType == EWarInventoryType::Skill || InData->InventoryType == EWarInventoryType::Consumable && InSlot->ItemDataInSlot.bIsEmpty)
		{
			InSlot->AddInventoryToSlot(InID);
			InSlot->ItemDataInSlot.ParentPanel = "Quick";
			break;
		}
	}
}
