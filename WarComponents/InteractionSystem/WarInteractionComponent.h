#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "War/WarComponents/InteractionSystem/DataType/WarInteractionData.h"
#include "WarInteractionComponent.generated.h"


class AWarHeroCharacter;
class AWarPlayerController;
class AInventoryBase;
class USphereComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WAR_API UWarInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="MyConfig")
	EInteractionType InteractionType = EInteractionType::None;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Inventory")
	TObjectPtr<USphereComponent> InteractionSphereComponent;
	// 焦点射线检测，当前可交互物体
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MyConfig")
	float CrosshairTraceDistance = 3000.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="MyConfig")
	AActor* CurrentInteractable = nullptr;
	FVector CameraOffset = FVector::ZeroVector;
	
	//参数设定
	FCollisionQueryParams TraceParams;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="MyConfig")
	bool bEnableCrosshairTrace = false;
	
	UWarInteractionComponent();
	//焦点射线检测
	void CrosshairTrace();
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION()
	virtual void EndOnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	UFUNCTION()
	virtual void BeginOnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
