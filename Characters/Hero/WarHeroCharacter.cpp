#include "WarHeroCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "InputActionValue.h"
#include "GameTags/WarGameTags.h"
#include "WarComponents/Input/WarInputComponent.h"
#include "WarComponents/InventorySystem/WarInventoryComponent.h"
#include "WarComponents/InteractionSystem/WarInteractionComponent.h"


AWarHeroCharacter::AWarHeroCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	WarInventoryComponent = CreateDefaultSubobject<UWarInventoryComponent>(TEXT("WarInventoryComponent"));
	WarInteractionComponent = CreateDefaultSubobject<UWarInteractionComponent>(TEXT("WarInteractionComponent"));
}


void AWarHeroCharacter::BeginPlay()
{
	Super::BeginPlay();
	checkf(WarInventoryComponent, TEXT("WarInventoryComponent is NULL"));
	checkf(WarInteractionComponent, TEXT("WarInteractionComponent is NULL"));
}


void AWarHeroCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	checkf(WarInputConfig, TEXT("WarInputConfig is Null"))
	if (UWarInputComponent* WarInputComponent = Cast<UWarInputComponent>(PlayerInputComponent))
	{
		WarInputComponent->BindNativeInputAction(WarInputConfig, WarGameTags::Input_Move, ETriggerEvent::Triggered, this, &ThisClass::Move);
		WarInputComponent->BindNativeInputAction(WarInputConfig, WarGameTags::Input_MouseLook, ETriggerEvent::Triggered, this, &ThisClass::Look);
		WarInputComponent->BindNativeInputAction(WarInputConfig, WarGameTags::Input_Look, ETriggerEvent::Triggered, this, &ThisClass::Look);
		WarInputComponent->BindNativeInputAction(WarInputConfig, WarGameTags::Input_Jump, ETriggerEvent::Started, this, &ThisClass::Jump);
		WarInputComponent->BindNativeInputAction(WarInputConfig, WarGameTags::Input_Jump, ETriggerEvent::Completed, this, &ThisClass::StopJumping);
		WarInputComponent->BindNativeInputAction(WarInputConfig, WarGameTags::Input_LightAttack, ETriggerEvent::Triggered, this, &ThisClass::LightAttack);
		WarInputComponent->BindNativeInputAction(WarInputConfig, WarGameTags::Input_HeavyAttack, ETriggerEvent::Triggered, this, &ThisClass::HeavyAttack);
		WarInputComponent->BindNativeInputAction(WarInputConfig, WarGameTags::Input_Inventory, ETriggerEvent::Completed, this, &ThisClass::ToggleInventoryUI);
		WarInputComponent->BindNativeInputAction(WarInputConfig, WarGameTags::Input_Character, ETriggerEvent::Completed, this, &ThisClass::ToggleCharacterUI);
	}
}


void AWarHeroCharacter::HeavyAttack(const FInputActionValue& Value)
{
}

void AWarHeroCharacter::LightAttack(const FInputActionValue& Value)
{
}

void AWarHeroCharacter::ToggleInventoryUI(const FInputActionValue& Value)
{
	if (WarInventoryComponent)
	{
		WarInventoryComponent->ToggleInventoryUI();
	}
}

void AWarHeroCharacter::ToggleCharacterUI(const FInputActionValue& Value)
{
	if (WarInventoryComponent)
	{
		WarInventoryComponent->ToggleCharacterUI();
	}
}

void AWarHeroCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	DoMove(MovementVector.X, MovementVector.Y);
}

void AWarHeroCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AWarHeroCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AWarHeroCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AWarHeroCharacter::DoJumpStart()
{
	Jump();
}

void AWarHeroCharacter::DoJumpEnd()
{
	StopJumping();
}


//设置计时器
void AWarHeroCharacter::StartCooldown(const float& CooldownTime)
{
	bCanClick = false;

	FTimerHandle CooldownTimerHandle;
	GetWorldTimerManager().SetTimer(
		CooldownTimerHandle,
		this,
		&ThisClass::ResetCooldown,
		CooldownTime,
		false
	);
}

//刷新计时器
void AWarHeroCharacter::ResetCooldown()
{
	bCanClick = true;
}
