
#include "WarInteractionComponent.h"


UWarInteractionComponent::UWarInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UWarInteractionComponent::BeginPlay()
{
	Super::BeginPlay();
}


void UWarInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

