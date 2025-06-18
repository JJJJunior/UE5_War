#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "WarComponents/InventorySystem/DynamicData/InventoryInstanceData.h"
#include "WarInventoryDataTableRow.generated.h"


UENUM(BlueprintType)
enum class EEquipmentSlotType:uint8
{
	None = 0,
	Head,
	Body,
	Neck,
	Legs,
	Footer,
	LeftHand,
	RightHand,
	Ring1,
	Ring2,
};

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
	EEquipmentSlotType EquipmentSlotType = EEquipmentSlotType::None;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FName SocketName;
};
