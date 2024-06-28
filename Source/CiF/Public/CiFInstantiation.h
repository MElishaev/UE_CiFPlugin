// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Utilities.h"
#include "UObject/Object.h"
#include "CiFInstantiation.generated.h"

/**
 * 
 */
UCLASS()
class CIF_API UCiFInstantiation : public UObject
{
	GENERATED_BODY()
public:
	UCiFInstantiation();
	
	bool requiresOtherToPerform() const;
public:
	IdType mId;

private:
	static UniqueIDGenerator mIDGenerator;
};
