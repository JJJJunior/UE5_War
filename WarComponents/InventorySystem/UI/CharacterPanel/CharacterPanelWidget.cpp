#include "CharacterPanelWidget.h"
#include "Components/SizeBox.h"
#include "GameInstance/WarGameInstanceSubSystem.h"
#include "War/WarComponents/InventorySystem/UI/ItemSlotWidget.h"
#include "War/Characters/Hero/WarHeroCharacter.h"
#include "WarComponents/InventorySystem/WarInventoryComponent.h"


void UCharacterPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	InitSlots();
	SyncSlots();
}

TObjectPtr<UDataTable> UCharacterPanelWidget::GetInventoryDataTable() const
{
	UWarGameInstanceSubSystem* Subsystem = GetWorld()->GetGameInstance()->GetSubsystem<UWarGameInstanceSubSystem>();
	if (!IsValid(Subsystem))
	{
		UE_LOG(LogTemp, Error, TEXT("WarSubsystem or its dependencies are invalid."));
		return nullptr;
	}
	return Subsystem->GetCachedWarInventoryDataTable();
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
			SlotWidget->RemoveItem();
		}
	}
}

// 装备同步
void UCharacterPanelWidget::SyncSlots()
{
	ClearAllSlots();

	AWarHeroCharacter* Character = Cast<AWarHeroCharacter>(GetOwningPlayerPawn());
	ensureMsgf(Character, TEXT("AWarHeroCharacter 不存在"));
	if (!Character)return;

	for (const auto& Item : Character->GetWarInventoryComponent()->GetCurrentEquippedItems())
	{
		// 直接传递 FInventoryInstanceData
		AddItemToSlot(Item);
	}
}

// 设置装备到指定槽位
void UCharacterPanelWidget::AddItemToSlot(const FGuid& InID)
{
	AWarHeroCharacter* Character = Cast<AWarHeroCharacter>(GetOwningPlayerPawn());
	ensureMsgf(Character, TEXT("AWarHeroCharacter 不存在"));
	if (!Character)return;
	const FInventoryInstanceData* InData = Character->GetWarInventoryComponent()->GetInventoryDataByGuid(InID);
	ensureMsgf(Character, TEXT("FInventoryInstanceData 不存在"));
	if (!InData)return;

	if (FWarInventoryRow* ItemRow = this->GetInventoryDataTable()->FindRow<FWarInventoryRow>(InData->TableRowID, "Find ItemName"))
	{
		if (ItemRow->InventoryType == EWarInventoryType::Equipment)
		{
			if (UItemSlotWidget** SlotWidgetPtr = CurrentCharacterSlots.Find(ItemRow->EquipmentSlotType))
			{
				if (UItemSlotWidget* SlotWidget = *SlotWidgetPtr)
				{
					SlotWidget->AddInventoryToSlot(InID);
				}
			}
		}
	}
}

// 移除装备
void UCharacterPanelWidget::RemoveItemFromSlot(const FGuid& InID)
{
	AWarHeroCharacter* Character = Cast<AWarHeroCharacter>(GetOwningPlayerPawn());
	ensureMsgf(Character, TEXT("AWarHeroCharacter 不存在"));
	if (!Character)return;
	const FInventoryInstanceData* InData = Character->GetWarInventoryComponent()->GetInventoryDataByGuid(InID);
	ensureMsgf(Character, TEXT("FInventoryInstanceData 不存在"));
	if (!InData)return;

	if (FWarInventoryRow* ItemRow = this->GetInventoryDataTable()->FindRow<FWarInventoryRow>(InData->TableRowID, "Find ItemName"))
	{
		if (UItemSlotWidget** SlotWidgetPtr = CurrentCharacterSlots.Find(ItemRow->EquipmentSlotType))
		{
			if (UItemSlotWidget* SlotWidget = *SlotWidgetPtr)
			{
				SlotWidget->RemoveItem();
			}
		}
	}
}
