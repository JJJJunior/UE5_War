#include "QuickPanelWidget.h"
#include "WarComponents/InventorySystem/UI/ItemSlotWidget.h"
#include "War/Characters/Hero/WarHeroCharacter.h"
#include "War/WarComponents/InventorySystem/WarInventoryComponent.h"
#include "Components/SizeBox.h"
#include "Components/HorizontalBox.h"
#include "Tools/MyLog.h"


class UWarGameInstanceSubSystem;

void UQuickPanelWidget::NativeConstruct()
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


void UQuickPanelWidget::InitSlots()
{
	if (!QuickSizeBox) return;

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
	const TArray<FItemInBagData>& CurrentQuickItems = CachedCharacter->GetWarInventoryComponent()->GetCurrentInQuickItems();
	if (CurrentQuickItems.Num() > 0)
	{
		for (const auto& InQuickItem : CurrentQuickItems)
		{
			AddItemToSlot(InQuickItem);
		}
	}
}

void UQuickPanelWidget::RemoveItemFromSlot(const FItemInBagData& InBagData)
{
	for (auto& InSlot : QuickSlots)
	{
		if (InSlot->ItemDataInSlot.bIsEmpty)
		{
			continue;
		}
		// 判断这个格子的 InstanceIDs 里是否包含目标实例 ID
		if (InSlot->ItemDataInSlot.InstanceID == InBagData.InstanceID.ToString())
		{
			// 执行移除
			InSlot->RemoveItemByInstanceID(InBagData);
			break; // 找到立即退出
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
void UQuickPanelWidget::AddItemToSlot(const FItemInBagData& InBagData)
{
	for (auto& InSlot : QuickSlots)
	{
		if (InBagData.InventoryType == EWarInventoryType::Skill || InBagData.InventoryType == EWarInventoryType::Consumable && InSlot->ItemDataInSlot.bIsEmpty)
		{
			InSlot->AddInventoryToSlot(InBagData);
			InSlot->ItemDataInSlot.ParentPanel = "Quick";
			break;
		}
	}
}
