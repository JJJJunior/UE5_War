#pragma once

#include "CoreMinimal.h"
#include "DataManager/EnumTypes/WarEnumTypes.h"
#include "GameFramework/Actor.h"
#include "War/WarComponents/InteractionSystem/Interface/InteractableInterface.h"
#include "War/WarComponents/PersistentSystem/Interface/WarSaveGameInterface.h"
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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FGuid PersistentID = FGuid();
	UPROPERTY()
	EInventoryWorldState CurrentState = EInventoryWorldState::None;

public:
	AInventoryBase();
	// 交互函数
	virtual void Interact_Implementation() override;
	// 瞄准进入时
	virtual void OnBeginFocus_Implementation() override;
	// 瞄准离开时
	virtual void OnEndFocus_Implementation() override;
	virtual void BeginPlay() override;

	virtual void SaveActorData(FMemoryWriter& MemoryWriter) const override;
	virtual void LoadActorData(FMemoryReader& MemoryReader) override;
	FORCEINLINE virtual FGuid GetPersistentID() const override { return PersistentID; }
	FORCEINLINE virtual void SetPersistentID(const FGuid& NewID) override { PersistentID = NewID; }
	FORCEINLINE virtual FName GetTableRowID() const override { return TableRowID; }
	FORCEINLINE EInventoryWorldState GetCurrentWorldState() const { return CurrentState; }

	void SetWorldState(const EInventoryWorldState& NewState);
	//根据状态调整碰撞体
	void UpdateInteractionCollision() const;
};
