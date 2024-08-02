// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CiFMicrotheory.generated.h"

class UCiFGameObject;
class UCiFCharacter;
class UCiFInfluenceRuleSet;
class UCiFRule;
class UCiFSocialExchange;
/**
 * Microtheory holds influence rules of initiator and responder for a specific type of rule.
 * The point of this class is to hold repetitive influence rules that makes common sense.
 * The theories that their rule evaluates to true is taken into account in the current social context
 * when forming volitions. Initiator's rules considered in his volitions and the responder's in the
 * his response.
 * 
 * @example friends(x,y) rule will hold influence rules that will make the two participating actors
 * less likely to fight and become enemies and more likely to want to hangout with each other.
 * This is a general behavior of friends no matter who are the participants. This frees the social exchange
 * to handle a more specific personal rules that are more based on the individual actors themselves and
 * the social context. 
 */
UCLASS()
class CIF_API UCiFMicrotheory : public UObject
{
	GENERATED_BODY()

public:

	UCiFMicrotheory();
	
	/**
	 * This function will score an influence rule set for all others that fit the definition or no others 
	 * if the definition doesn't require it.
	 * 
	 * @return The total weight of the influence rules
	 */
	float score(UCiFCharacter* initiator,
	            UCiFGameObject* responder,
	            UCiFSocialExchange* se,
	            TArray<UCiFGameObject*>& others) const;

	static UCiFMicrotheory* loadFromJson(TSharedPtr<FJsonObject> json, const UObject* worldContextObject);

public:
	FName mName = ""; // name of micro theory (acts also as ID due to being FName)

	UPROPERTY()
	UCiFRule* mDefinition; // the condition we want to represent with influence rule. often just one Predicate

	UCiFInfluenceRuleSet* mInitiatorIR;
	UCiFInfluenceRuleSet* mResponderIR;
};
