#pragma once

#include "CoreMinimal.h"
#include "War/Characters/WarCharacterBase.h"
#include "Logging/LogMacros.h"
#include "WarHeroCharacter.generated.h"


class UWarAbilityComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;
class UDataAsset_InputConfig;
class UWarInventoryComponent;
class UInventoryPanelWidget;

UCLASS(Blueprintable)
class AWarHeroCharacter : public AWarCharacterBase
{
	GENERATED_BODY()
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* JumpAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* MoveAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* LookAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* MouseLookAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* HeavyAttackAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* LightAttackAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* InventoryAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* CharacterAction;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="InputData")
	UDataAsset_InputConfig* WarInputConfig;

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void HeavyAttack(const FInputActionValue& Value);
	void LightAttack(const FInputActionValue& Value);
	void ToggleInventoryUI(const FInputActionValue& Value);
	void ToggleCharacterUI(const FInputActionValue& Value);

	void StartCooldown(const float& CooldownTime);
	bool bCanClick = true;
	void ResetCooldown();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MyComponents")
	TObjectPtr<UWarInventoryComponent> WarInventoryComponent;

public:
	AWarHeroCharacter();

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintCallable)
	UWarInventoryComponent* GetWarInventoryComponent() const { return WarInventoryComponent; }
};
