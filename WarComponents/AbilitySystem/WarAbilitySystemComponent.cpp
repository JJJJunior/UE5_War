#include "WarAbilitySystemComponent.h"

UWarAbilitySystemComponent::UWarAbilitySystemComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicated(true);
}

void UWarAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();
}
