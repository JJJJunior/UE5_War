#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "War/WarComponents/InventorySystem/Interface/ContainerWidgetInterface.h"
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
class WAR_API UInventoryPanelWidget : public UUserWidget, public IContainerWidgetInterface
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category="UI")
	USizeBox* InventorySizeBox;
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category="UI")
	UBorder* BackgroundBorder;
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category="UI")
	UWrapBox* InventoryWrapBox;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI")
	TSubclassOf<UItemSlotWidget> ItemSlotWidgetClass;
	UPROPERTY(VisibleAnywhere, Category="UI")
	TArray<TObjectPtr<UItemSlotWidget>> CurrentInventorySlots;
	UPROPERTY(VisibleAnywhere, Category="UI")
	int32 MaxSlots = 72;
	UPROPERTY()
	TWeakObjectPtr<AWarHeroCharacter> CachedCharacter;

	virtual void NativeConstruct() override;
	virtual void InitSlots() override;
	virtual void SyncSlots() override;
	virtual void RemoveItemFromSlot(const FGuid& InID) override;
	virtual void ClearAllSlots() override;
	virtual void AddItemToSlot(const FGuid& InID) override;
	virtual int32 GetMaxSlots() const override { return MaxSlots; }
	UItemSlotWidget* FindSuitableSlot(const FName& TableRowID);
	UItemSlotWidget* FindFirstEmptySlot();
};
