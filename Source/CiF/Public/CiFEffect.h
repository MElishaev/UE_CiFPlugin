// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Utilities.h"
#include "UObject/Object.h"
#include "CiFEffect.generated.h"

class UCiFRule;
/**
 * 
 */
UCLASS()
class CIF_API UCiFEffect : public UObject
{
	GENERATED_BODY()

public:
	UCiFEffect();

	
public:
	uint32 mId;
	UCiFRule* mCondition; // condition for if this effect can be happen
	UCiFRule* mChange; // the rule containing the social change associated with the effect

private:

	static UniqueIDGenerator mIDGenerator;
	
};
