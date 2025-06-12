#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "WarGameInstanceSubSystem.generated.h"


UCLASS()
class WAR_API UWarGameInstanceSubSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UDataTable> WarSpawnDataTable;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UDataTable> CachedWarSpawnDataTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UDataTable> WarAbilityDataTable;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UDataTable> CachedWarAbilityDataTable;

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	TObjectPtr<UDataTable> GetCachedWarInventoryDataTable() const;
	TObjectPtr<UDataTable> GetCachedWarAbilityDataTable() const;
};
