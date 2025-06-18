#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "WarGameInstanceSubSystem.generated.h"

class UGameConfigData;
class UTexture2D;
class UWarDataManager;

UCLASS()
class WAR_API UWarGameInstanceSubSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	TObjectPtr<UDataTable> GetCachedWarInventoryDataTable() const;
	TObjectPtr<UDataTable> GetCachedWarAbilityDataTable() const;
	TSoftObjectPtr<UMaterialInstance> GetOutLineMaterial() const;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ConfigData")
	TObjectPtr<UGameConfigData> GameConfigData;
	UPROPERTY()
	TWeakObjectPtr<UWarDataManager> WarDataManager;

	FORCEINLINE TWeakObjectPtr<UWarDataManager> GetWarDataManager() const { return WarDataManager; }
};
