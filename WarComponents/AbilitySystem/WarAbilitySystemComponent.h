﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "AbilitySystemComponent.h"
#include "WarAbilitySystemComponent.generated.h"

UCLASS()
class WAR_API UWarAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UWarAbilitySystemComponent();
	virtual void BeginPlay() override;
};
