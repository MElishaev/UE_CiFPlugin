// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CiFRule.h"
#include "CiFInfluenceRule.generated.h"

/**
 * 
 */
UCLASS()
class CIF_API UCiFInfluenceRule : public UCiFRule
{
	GENERATED_BODY()

public:

	static UCiFInfluenceRule* loadFromJson(TSharedPtr<FJsonObject> ruleJson);
	
public:
	int8 mWeight; // the weight associated with the influence rule
	int8 mEvaluatedWeight; // the weight of the rule evaluated in a microtheory + cast context.
						   // this will only be different than the weight if it is a rule that includes an other role
};
