#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameConfigData.generated.h"


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
};
