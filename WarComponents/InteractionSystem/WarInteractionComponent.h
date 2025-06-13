
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WarInteractionComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WAR_API UWarInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWarInteractionComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
