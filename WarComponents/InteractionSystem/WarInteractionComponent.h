#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interface/InteractableInterface.h"
#include "War/WarComponents/InteractionSystem/DataType/WarInteractionData.h"
#include "WarInteractionComponent.generated.h"


class AWarHeroCharacter;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WAR_API UWarInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MyConfig")
	float InteractorRange = 300.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MyConfig")
	float InteractionRadius = 10.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MyConfig")
	EInteractionType InteractionType = EInteractionType::None;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MyConfig")
	TObjectPtr<AWarHeroCharacter> CachedWarHeroCharacter;
	// 当前瞄准的可交互物体
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="MyConfig")
	AActor* CurrentInteractable = nullptr;

public:
	UWarInteractionComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void TryInteract();
};
