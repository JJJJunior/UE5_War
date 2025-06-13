#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "War/WarComponents/InteractionSystem/Interface/InteractableInterface.h"
#include "InventoryBase.generated.h"


class UStaticMeshComponent;
class USphereComponent;

UCLASS()
class WAR_API AInventoryBase : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

protected:
	UPROPERTY()
	TObjectPtr<USceneComponent> RootScene;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Inventory")
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Inventory")
	TObjectPtr<USphereComponent> InteractorSphereComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Inventory")
	FName TableRowID;

public:
	AInventoryBase();
	// 交互函数
	virtual void Interact_Implementation(AWarHeroCharacter* Interactor) override;
	// 瞄准进入时
	virtual void OnBeginFocus_Implementation() override;
	// 瞄准离开时
	virtual void OnEndFocus_Implementation() override;
	//文字提示
	virtual FString GetInteractText_Implementation() const;
};
