// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CiFSubsystem.generated.h"

class UCiFManager;
/**
 * 
 */
UCLASS()
class CIF_API UCiFSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	UCiFManager* getInstance();

private:

	UPROPERTY()
	UCiFManager* mCiFInstance = nullptr;	
};
