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

	UFUNCTION(BlueprintCallable)
	UCiFManager* getInstance();

	UFUNCTION(BlueprintCallable)
	UCifImplementationBase* getImplementation() const;

	UFUNCTION(BlueprintCallable)
	void setImplementation(UCifImplementationBase* impl);
	
private:
	UPROPERTY()
	UCifImplementationBase* mImpl = nullptr; // holds the game specific implementation related to cif
	
	UPROPERTY()
	UCiFManager* mCiFInstance = nullptr;
};
