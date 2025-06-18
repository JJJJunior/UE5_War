#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "WarGameInstanceSubSystem.generated.h"

class UGameConfigData;
class UTexture2D;
class UWarDataManager;
class UWarPersistentSystem;

UCLASS()
class WAR_API UWarGameInstanceSubSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

protected:
	UPROPERTY()
	TObjectPtr<UWarPersistentSystem> WarPersistentSystem;

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	TObjectPtr<UDataTable> GetCachedWarInventoryDataTable() const;
	TObjectPtr<UDataTable> GetCachedWarAbilityDataTable() const;
	TSoftObjectPtr<UMaterialInstance> GetOutLineMaterial() const;
	UPROPERTY()
	TObjectPtr<UGameConfigData> GameConfigData;
	UFUNCTION(BlueprintCallable)
	UWarPersistentSystem* GetWarPersistentSystem() const { return WarPersistentSystem; }
};
