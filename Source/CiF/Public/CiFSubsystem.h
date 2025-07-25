// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CifImplementationBase.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CiFSubsystem.generated.h"

class UCifImplementationBase;
class UCiFManager;
/**
 * 
 */
UCLASS()
class CIF_API UCiFSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

    // todo - maybe i need to hide it and access only through implementation?
	UFUNCTION(BlueprintCallable, Category = "CiF")
	UCiFManager* getInstance();

	UFUNCTION(BlueprintCallable, Category = "CiF")
	UCifImplementationBase* getImplementation() const;

	UFUNCTION(BlueprintCallable, Category = "CiF")
	void setImplementation(UCifImplementationBase* impl);
	
private:
	UPROPERTY()
	UCifImplementationBase* mImpl = nullptr; // holds the game specific implementation related to cif
	
	UPROPERTY()
	UCiFManager* mCiFInstance = nullptr;
};
