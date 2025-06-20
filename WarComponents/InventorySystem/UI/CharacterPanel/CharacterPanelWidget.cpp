#include "CharacterPanelWidget.h"
#include "Components/SizeBox.h"
#include "GameInstance/WarGameInstanceSubSystem.h"
#include "Tools/MyLog.h"
#include "War/WarComponents/InventorySystem/UI/ItemSlotWidget.h"
#include "War/Characters/Hero/WarHeroCharacter.h"
#include "WarComponents/InventorySystem/WarInventoryComponent.h"


void UCharacterPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	checkf(ItemSlotWidgetClass, TEXT("ItemSlotWidgetClass 没有配置"));

	CachedCharacter = Cast<AWarHeroCharacter>(GetOwningPlayerPawn());
	if (!CachedCharacter.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("CachedCharacter 弱指针无效"));
		return;
	}

	InitSlots();
	SyncSlots();
}


// 初始化所有装备槽
void UCharacterPanelWidget::InitSlots()
{
	TMap<EEquipmentSlotType, USizeBox*> SlotBoxMap = {
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
			// print(TEXT("装备槽位 [%d] 已初始化."), static_cast<uint8>(SlotType));
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
	if (!CachedCharacter.IsValid()) return;

	ClearAllSlots();

	for (const auto& Item : CachedCharacter->GetWarInventoryComponent()->GetCurrentEquippedItems())
	{
		AddItemToSlot(Item);
	}
}


// 设置装备到指定槽位
void UCharacterPanelWidget::AddItemToSlot(const FItemInBagData& InBagData)
{
	if (!CachedCharacter.IsValid()) return;
	const FWarInventoryRow* ItemRow = UWarGameInstanceSubSystem::FindInventoryRow(this, InBagData.TableRowID);

	if (InBagData.InventoryType != EWarInventoryType::Armor && InBagData.InventoryType != EWarInventoryType::Weapon)
	{
		print(TEXT("AddItemToSlot: 非装备类型，忽略。"));
		return;
	}
	if (UItemSlotWidget** SlotWidgetPtr = CurrentCharacterSlots.Find(ItemRow->EquipmentSlotType))
	{
		if (UItemSlotWidget* SlotWidget = *SlotWidgetPtr)
		{
			// 装备槽位只允许一个物品，先清空再放入
			SlotWidget->AddInventoryToSlot(InBagData);
		}
	}
}

// 移除装备
void UCharacterPanelWidget::RemoveItemFromSlot(const FItemInBagData& InBagData)
{
	if (!CachedCharacter.IsValid()) return;

	const FWarInventoryRow* ItemRow = UWarGameInstanceSubSystem::FindInventoryRow(this, InBagData.TableRowID);

	if (!ItemRow || ItemRow->InventoryType != EWarInventoryType::Weapon && ItemRow->InventoryType != EWarInventoryType::Armor)
	{
		print(TEXT("RemoveItemFromSlot: 非装备类型，忽略。"));
		return;
	}

	if (UItemSlotWidget** SlotWidgetPtr = CurrentCharacterSlots.Find(ItemRow->EquipmentSlotType))
	{
		if (UItemSlotWidget* SlotWidget = *SlotWidgetPtr)
		{
			SlotWidget->RemoveItemByInstanceID(InBagData);
		}
	}
}
