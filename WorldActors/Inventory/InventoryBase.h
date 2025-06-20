#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "War/WarComponents/InteractionSystem/Interface/InteractableInterface.h"
#include "War/SaveGame/Interface/WarSaveGameInterface.h"
#include "InventoryBase.generated.h"


class UStaticMeshComponent;
class USphereComponent;

UCLASS()
class WAR_API AInventoryBase : public AActor, public IInteractableInterface, public IWarSaveGameInterface
{
	GENERATED_BODY()

protected:
	UPROPERTY()
	TObjectPtr<USceneComponent> RootScene;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Inventory")
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent; //主要用于WarInteractionComponent的射线进行交互
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Inventory")
	TObjectPtr<USphereComponent> InteractionSphere; //主要用于和WarInteractionComponent进行overlap交互
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Inventory")
	FName TableRowID;
	UPROPERTY()
	FGuid PersistentActorID;

public:
	AInventoryBase();
	// 交互函数
	virtual void Interact_Implementation() override;
	// 瞄准进入时
	virtual void OnBeginFocus_Implementation() override;
	// 瞄准离开时
	virtual void OnEndFocus_Implementation() override;
	virtual void BeginPlay() override;
	//关闭碰撞体
	void DisableInteractionSphere() const;
	FName GetTableRowID() const { return TableRowID; }

	virtual void SaveActorData(FMemoryWriter& MemoryWriter) const override;
	virtual void LoadActorData(FMemoryReader& MemoryReader) const override;
	virtual FGuid GetPersistentActorID() const override { return PersistentActorID; };
};
