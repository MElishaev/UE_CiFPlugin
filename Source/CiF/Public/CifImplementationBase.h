// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CifImplementationBase.generated.h"

class UCiFSubsystem;
class UCiFManager;
/**
 * This class acts as a base class for game specific implementations related to cif system.
 * For example, if your game needs to compile a vector of strings of social game names to
 * present for the player as options to choose - this considered to be game specific and should
 * be implemented in a derived class of this
 */
UCLASS(Abstract)
class CIF_API UCifImplementationBase : public UObject
{
	GENERATED_BODY()
	
protected:

	friend UCiFSubsystem;
	
	UPROPERTY(BlueprintReadOnly)
	UCiFManager* mCifManager = nullptr;
};
