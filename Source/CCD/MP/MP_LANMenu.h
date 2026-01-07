// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MP_LANMenu.generated.h"

class UEditableTextBox;
class UButton;

UCLASS()
class CCD_API UMP_LANMenu : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeOnInitialized() override;
	
private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr <UEditableTextBox> TextBox_IPAddress;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr <UButton> Button_Host;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr <UButton> Button_Join;
	
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UWorld> HostingLevel;
	
	// Callback for Buttons
	UFUNCTION()
	void HostButtonClicked();
	
	UFUNCTION()
	void JoinButtonClicked();
};
