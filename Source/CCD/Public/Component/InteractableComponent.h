
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interface/InteractInterface.h"
#include "InteractableComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CCD_API UInteractableComponent : public UActorComponent, public IInteractInterface
{
	GENERATED_BODY()

public:	
	UInteractableComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
protected:
	virtual void BeginPlay() override;

public:	
	virtual void Interact_Implementation(AActor* Interactor) override;
};