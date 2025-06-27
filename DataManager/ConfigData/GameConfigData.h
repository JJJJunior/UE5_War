#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameConfigData.generated.h"


class URootPanelWidget;
class UInventoryPanelWidget;

UCLASS()
class WAR_API UGameConfigData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameConfig")
	TSoftObjectPtr<UDataTable> WarInventoryDataTable;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameConfig")
	TSoftObjectPtr<UDataTable> WarAbilityDataTable;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameConfig")
	TSoftObjectPtr<UMaterialInstance> OutLineMaterial;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameConfig")
	FVector FollowCameraOffset;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameConfig")
	FVector FollowCameraNormal;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameConfig")
	FString StaticPlayerID;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GameConfig")
	TSubclassOf<URootPanelWidget> RootPanelWidgetClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GameConfig")
	TSubclassOf<UInventoryPanelWidget> InventoryPanelWidgetClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GameConfig")
	int32 MaxSlots;
};
