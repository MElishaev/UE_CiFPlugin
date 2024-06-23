// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CiFInfluenceRuleSet.generated.h"

class UCiFCharacter;
class UCiFInfluenceRule;
/**
 * 
 */
UCLASS()
class CIF_API UCiFInfluenceRuleSet : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Scores the rules of the influence rule set and returns the aggregate
	 * weight of the true influence rules
	 *
	 * @param	initiator	The initiator of the social game.
	 * @param	responder	The responder of the social game.
	 * @param	other		A third party in the social game.
	 * @return The sum of the weight values associated with true influence
	 * rules.
	 */
	float scoreRules(UCiFCharacter* initiator,
	                 UCiFGameObject* responder,
	                 UCiFGameObject* other = nullptr,
	                 UCiFSocialExchange* se = nullptr,
	                 TArray<UCiFGameObject*> activeOtherCast = {},
	                 FName microtheoryName = "",
	                 bool isResponder = false);

	/**
	 * Scores the rules of the influence rule set and returns the aggregate
	 * weight of the true influence rules by going through all others whenever
	 * others are relevant
	 *
	 * @param	initiator	The initiator of the social game.
	 * @param	responder	The responder of the social game.
	 * @param	other		A third party in the social game.
	 * @return The sum of the weight values associated with true influence
	 * rules.
	 */
	float scoreRulesWithVariableOther(UCiFCharacter* initiator,
	                                  UCiFGameObject* responder,
	                                  UCiFGameObject* other = nullptr,
	                                  UCiFSocialExchange* se = nullptr,
	                                  TArray<UCiFGameObject*> activeOtherCast = {},
	                                  FName microtheoryName = "",
	                                  bool isResponder = false);

public:
	TArray<UCiFInfluenceRule*> mInfluenceRules;
};
