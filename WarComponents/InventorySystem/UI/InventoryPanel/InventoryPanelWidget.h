#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DataManager/DynamicData/InventoryData.h"
#include "DataManager/EnumTypes/WarEnumTypes.h"
#include "InventoryPanelWidget.generated.h"


class UCanvasPanel;
class UWrapBox;
class UItemSlotWidget;
class UBackgroundBlur;
class UBorder;
class AInventoryBase;
class USizeBox;
class AWarHeroCharacter;

UCLASS()
class WAR_API UInventoryPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category="UI")
	USizeBox* HeadBox;
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category="UI")
	USizeBox* BodyBox;
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category="UI")
	USizeBox* NeckBox;
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category="UI")
	USizeBox* LegBox;
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category="UI")
	USizeBox* FooterBox;
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category="UI")
	USizeBox* LeftHandBox;
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category="UI")
	USizeBox* RightHandBox;
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category="UI")
	USizeBox* RingBox1;
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category="UI")
	USizeBox* RingBox2;
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category="UI")
	UWrapBox* InventoryWrapBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UI")
	TSubclassOf<UItemSlotWidget> ItemSlotWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UI")
	TMap<ESlotType, USizeBox*> CharacterSlots;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UI")
	TArray<TObjectPtr<UItemSlotWidget>> QuickSlots;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UI")
	TArray<TObjectPtr<UItemSlotWidget>> InventorySlots;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UI")
	int32 MaxSlots = 24;
	UPROPERTY()
	TWeakObjectPtr<AWarHeroCharacter> CachedCharacter;

	virtual void NativeConstruct() override;
	virtual void InitSlots();
	virtual void SyncSlots();
	virtual void RemoveItemFromSlot(const FInventoryItemInDB& InItemInDB);
	virtual void ClearAllSlots();
	virtual void AddItemToSlot(const FInventoryItemInDB& InItemInDB);
	virtual int32 GetMaxSlots() const { return MaxSlots; }
	UItemSlotWidget* FindSuitableSlot(const FName& TableRowID);
	UItemSlotWidget* FindFirstEmptySlot();
};
