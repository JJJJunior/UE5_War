#include "CharacterPanelWidget.h"
#include "Components/SizeBox.h"
#include "War/WarComponents/InventorySystem/UI/ItemSlotWidget.h"
#include "War/Characters/Hero/WarHeroCharacter.h"
#include "WarComponents/InventorySystem/WarInventoryComponent.h"


void UCharacterPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	InitSlots();
	SyncSlots();
	CachedCharacter = Cast<AWarHeroCharacter>(GetOwningPlayerPawn());
}


// 初始化所有装备槽
void UCharacterPanelWidget::InitSlots()
{
	checkf(ItemSlotWidgetClass, TEXT("ItemSlotWidgetClass 没有配置"));

	const TMap<EEquipmentSlotType, USizeBox*> SlotBoxMap = {
		{EEquipmentSlotType::Head, HeadBox},
		{EEquipmentSlotType::Body, BodyBox},
		{EEquipmentSlotType::LeftHand, LeftHandBox},
		{EEquipmentSlotType::RightHand, RightHandBox},
		{EEquipmentSlotType::Legs, LegBox},
		{EEquipmentSlotType::Footer, FooterBox},
		{EEquipmentSlotType::Neck, NeckBox},
		{EEquipmentSlotType::Ring1, RingBox1},
		{EEquipmentSlotType::Ring2, RingBox2},
	};

	int32 index = 0;

	for (const auto& Pair : SlotBoxMap)
	{
		EEquipmentSlotType SlotType = Pair.Key;
		USizeBox* SlotBox = Pair.Value;

		if (!SlotBox) continue;

		if (UItemSlotWidget* SlotWidget = CreateWidget<UItemSlotWidget>(GetWorld(), ItemSlotWidgetClass))
		{
			//设置溯源
			SlotWidget->ItemDataInSlot.ParentPanel = "Character";
			SlotWidget->ItemDataInSlot.EquipmentSlotType = Pair.Key;
			//设置索引
			SlotWidget->ItemDataInSlot.SlotIndex = index;
			SlotWidget->SetVisibility(ESlateVisibility::Visible);
			SlotBox->AddChild(SlotWidget);
			CurrentCharacterSlots.Emplace(SlotType, SlotWidget);
			// 可选：Debug 显示
			// UE_LOG(LogTemp, Warning, TEXT("装备槽位 [%d] 已初始化."), static_cast<uint8>(SlotType));
		}
		index++;
	}
}

// 清空所有槽位
void UCharacterPanelWidget::ClearAllSlots()
{
	for (auto& Elem : CurrentCharacterSlots)
	{
		if (UItemSlotWidget* SlotWidget = Elem.Value)
		{
			SlotWidget->CleanSlot(); // 这里推荐用彻底清空的方法
		}
	}
}

// 装备同步
void UCharacterPanelWidget::SyncSlots()
{
	if (!CachedCharacter) return;

	ClearAllSlots();

	for (const auto& Item : CachedCharacter->GetWarInventoryComponent()->GetCurrentEquippedItems())
	{
		AddItemToSlot(Item);
	}
}


// 设置装备到指定槽位
void UCharacterPanelWidget::AddItemToSlot(const FGuid& InID)
{
	if (!CachedCharacter) return;

	const FWarInventoryRow* ItemRow = CachedCharacter->GetWarInventoryComponent()->FindItemRowByGuid(InID);
	if (!ItemRow || ItemRow->InventoryType != EWarInventoryType::Equipment)
	{
		UE_LOG(LogTemp, Warning, TEXT("AddItemToSlot: 非装备类型，忽略。"));
		return;
	}

	if (UItemSlotWidget** SlotWidgetPtr = CurrentCharacterSlots.Find(ItemRow->EquipmentSlotType))
	{
		if (UItemSlotWidget* SlotWidget = *SlotWidgetPtr)
		{
			// 装备槽位只允许一个物品，先清空再放入
			SlotWidget->AddInventoryToSlot(InID);
		}
	}
}

// 移除装备
void UCharacterPanelWidget::RemoveItemFromSlot(const FGuid& InID)
{
	if (!CachedCharacter) return;

	const FWarInventoryRow* ItemRow = CachedCharacter->GetWarInventoryComponent()->FindItemRowByGuid(InID);
	if (!ItemRow || ItemRow->InventoryType != EWarInventoryType::Equipment)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveItemFromSlot: 非装备类型，忽略。"));
		return;
	}

	if (UItemSlotWidget** SlotWidgetPtr = CurrentCharacterSlots.Find(ItemRow->EquipmentSlotType))
	{
		if (UItemSlotWidget* SlotWidget = *SlotWidgetPtr)
		{
			SlotWidget->RemoveItemByInstanceID(InID);
		}
	}
}
