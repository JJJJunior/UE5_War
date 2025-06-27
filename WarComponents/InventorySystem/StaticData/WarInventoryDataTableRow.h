#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "War/DataManager/EnumTypes/WarEnumTypes.h"
#include "WarInventoryDataTableRow.generated.h"


class AInventoryBase;
class UAnimMontage;
class UTexture2D;

USTRUCT(BlueprintType)
struct FWarInventoryRow : public FTableRowBase
{
	GENERATED_BODY()

	FWarInventoryRow(): Texture(nullptr)
	{
	}

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FString Description;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSoftClassPtr<AInventoryBase> InventorySoftClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Texture;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	EWarInventoryType InventoryType = EWarInventoryType::None;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	ESlotType SlotType = ESlotType::None;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FName SocketName;
};
