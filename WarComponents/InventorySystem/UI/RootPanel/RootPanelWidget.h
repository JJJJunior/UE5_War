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
class UCanvasPanelSlot;

UCLASS()
class WAR_API URootPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category="UI")
	UBackgroundBlur* BackgroundBlur;
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category="UI")
	UBorder* BackgroundBorder;
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category="UI")
	UCanvasPanel* RootContainer;
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category="UI")
	UWrapBox* CharacterWrapBox;
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category="UI")
	UWrapBox* InventoryWrapBox;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI")
	TSubclassOf<UInventoryPanelWidget> InventoryPanelWidgetClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI")
	TSubclassOf<UCharacterPanelWidget> CharacterPanelWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UI")
	TObjectPtr<UInventoryPanelWidget> InventoryPanelWidget;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UI")
	TObjectPtr<UCharacterPanelWidget> CharacterPanelWidget;

	virtual void NativeConstruct() override;
};
