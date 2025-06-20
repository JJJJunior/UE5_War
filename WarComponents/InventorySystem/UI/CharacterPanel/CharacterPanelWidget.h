#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WarComponents/InventorySystem/Interface/ContainerWidgetInterface.h"
#include "War/WarComponents/InventorySystem/StaticData/WarInventoryDataTableRow.h"
#include "CharacterPanelWidget.generated.h"


class UCanvasPanel;
class UWrapBox;
class USizeBox;
class UItemSlotWidget;
class UBorder;
class AWarHeroCharacter;


UCLASS()
class WAR_API UCharacterPanelWidget : public UUserWidget, public IContainerWidgetInterface
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category="UI")
	USizeBox* CharacterSizBox;
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category="UI")
	UBorder* BackgroundBorder;
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category="UI")
	UCanvasPanel* SizeBoxContainer;
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
	UPROPERTY()
	TWeakObjectPtr<AWarHeroCharacter> CachedCharacter;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI")
	TSubclassOf<UItemSlotWidget> ItemSlotWidgetClass;
	UPROPERTY(VisibleAnywhere, Category="UI")
	TMap<EEquipmentSlotType, UItemSlotWidget*> CurrentCharacterSlots;

	virtual void NativeConstruct() override;

	virtual void InitSlots() override;
	virtual void SyncSlots() override;
	virtual void RemoveItemFromSlot(const FItemInBagData& InBagData) override;
	virtual void ClearAllSlots() override;
	virtual void AddItemToSlot(const FItemInBagData& InBagData) override;
	virtual int32 GetMaxSlots() const override { return CurrentCharacterSlots.Num(); }
};
