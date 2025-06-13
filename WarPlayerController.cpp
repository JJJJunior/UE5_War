#include "WarPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Characters/Hero/WarHeroCharacter.h"
#include "WarComponents/InventorySystem/WarInventoryComponent.h"

void AWarPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
		{
			Subsystem->AddMappingContext(CurrentContext, 0);
		}
	}
}

void AWarPlayerController::BeginPlay()
{
	Super::BeginPlay();

	AWarHeroCharacter* CurrentCharacter = Cast<AWarHeroCharacter>(GetCharacter());
	CurrentCharacter->GetWarInventoryComponent()->OnUIStateChanged.AddDynamic(this, &ThisClass::RefreshInputMode);
}


void AWarPlayerController::RefreshInputMode(bool bInventoryVisible, bool bCharacterVisible)
{
	UE_LOG(LogTemp, Warning, TEXT("bInventoryVisible %d  bCharacterVisible %d"), bInventoryVisible, bCharacterVisible);
	if (!bInventoryVisible && !bCharacterVisible)
	{
		SetShowMouseCursor(false);
		FInputModeGameOnly InputMode;
		InputMode.SetConsumeCaptureMouseDown(false);
		SetInputMode(InputMode);

		UE_LOG(LogTemp, Warning, TEXT("Input Mode: GameOnly"));
	}
	else
	{
		SetShowMouseCursor(true);
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		SetInputMode(InputMode);

		UE_LOG(LogTemp, Warning, TEXT("Input Mode: GameAndUI"));
	}
}
