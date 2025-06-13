// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "War/WorldActors/Inventory/InventoryBase.h"
#include "WarWeaponBase.generated.h"


class UBoxComponent;

UCLASS()
class WAR_API AWarWeaponBase : public AInventoryBase
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Collision")
	TObjectPtr<UBoxComponent> WeaponCollisionBox;
	
public:
	AWarWeaponBase();
	UBoxComponent*GetWeaponCollisionBox()const;
};
