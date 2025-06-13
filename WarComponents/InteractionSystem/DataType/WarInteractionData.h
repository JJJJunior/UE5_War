#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EInteractionType : uint8
{
	None UMETA(DisplayName = "None"),
	Pickup UMETA(DisplayName = "Pickup"),
	OpenDoor UMETA(DisplayName = "Open Door"),
	Equip UMETA(DisplayName = "Equip"),
	Talk UMETA(DisplayName = "Talk"),
	UseVehicle UMETA(DisplayName = "Use Vehicle"),
	Custom UMETA(DisplayName = "Custom")
};
