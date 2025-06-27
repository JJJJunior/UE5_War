// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RootPanelWidget.generated.h"

class UCanvasPanel;
class UBackgroundBlur;
class UBorder;
class UWrapBox;
class UInventoryPanelWidget;
class UCharacterPanelWidget;
class UQuickPanelWidget;
class UCanvasPanelSlot;

UCLASS()
class WAR_API URootPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category="UI")
	UWrapBox* InventoryWrapBox;
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category="UI")
	UUserWidget* TargetWidget;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UI")
	TObjectPtr<UInventoryPanelWidget> InventoryPanelWidget;

	virtual void NativeConstruct() override;
};
