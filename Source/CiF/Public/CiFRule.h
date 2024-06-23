// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CiFCharacter.h"
#include "CiFRule.generated.h"

class UCiFSocialExchange;
class UCiFCharacter;
class UCiFPredicate;
/**
 * could be influence rule or could be trigger rule (both influences what other characters' volition's).
 *
 * influence rule structure is:
 * <condition> → <increment/decrement volition for an intent>
 * this means that if the condition is met, it modifies the score by the weight for the volition this rule is connected to.
 * 
 * For the Annoy social exchange in Prom Week, whose intent is to decrease the responder’s friendship feelings for the 
 * initiator, two of the 18 influence rules are:
 * network(romance, I, R) > 66 && trait(I, inarticulate) → +3
 * [SFKBLabel(cool, R, I) window(10)] → -3
 *
 * trigger rules can happen as an after effect of social exchange, due to the SE changing the
 * social status, a trigger rule can fire due to this change and apply some additional changes.
 * example:
 * A rule can say that if x and y aren’t enemies, and x has one of the traits that falls into the 
 * more general category of “nice” traits, and a third party z has done something that falls 
 * into the more general category of negative interactions to y in the last 7 social exchanges, 
 * and x hasn’t themself done something mean to y in the last 7 exchanges, then x gains the 
 * status of pitying y. Or, more succinctly, nice people will pity those who have mean things 
 * happen to them as long as they haven’t done mean things themselves to the same person. 
 * This rule, like all trigger rules, can fire after any social exchange.
 */
UCLASS()
class CIF_API UCiFRule : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Determines if the rule requires 3rd character to be valuated or evaluated.
	 * @return True iff 3rd character is required for processing the rule
	 */
	UFUNCTION(BlueprintCallable)
	bool isOtherCharacterRequired();

	/**
	 * Returns the conjunction of all the truth values of the Predicates
	 * that compose the rules. 
	 * 
	 * @param	initiator	The initiator of the social game.
	 * @param	responder	The responder of the social game.
	 * @param	other		A third party in the social game.
	 * @return True if all the predicates in the rules are true. Otherwise,
	 * false.
	 */
	bool evaluate(UCiFCharacter* initiator, UCiFGameObject* responder, UCiFGameObject* other = nullptr, UCiFSocialExchange* se = nullptr);

private:
	/**
	 * Determines the highest SFDB order of the predicates in this rule.
	 * @return The value of the highest SFDB order of this rule.
	 */
	int32 getHighestSFDBOrder();

	/**
	 * Evaluates a rule with respect to the time order specified in the predicates of the rule. 
	 * All rules with a sfdbOrder less than 1 are evaluated without temporal ordering constraints.
	 * 
	 * This function tolerates gaps in order meaning a rule can have predicates of orders 0, 3, 9, 100
	 * and this function will ignore the missing orders.
	 * 
	 * If there are multiple predicates of the same order in the rule, they must all be true after the next
	 * lowest order and before the next highest order. Any predicate of the same order is considered true as
	 * long as they are true in this time interval.
	 * 
	 * @param	primary		Primary character.
	 * @param	secondary	Secondary character.
	 * @param	tertiary	Tertiary character.
	 * @return	True if the rule is true when evaluated for the specific character binding wrt
	 * the time ordering of the Predicates in the rule.
	 */
	bool evaluateTimeOrderedRule(UCiFGameObject* primary, UCiFGameObject* secondary, UCiFGameObject* tertiary);
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<UCiFPredicate*> mPredicates; // the array of predicates that comprise this rule

	int8 mWeight;
	uint8 mLastTrueCount; // the number of predicate that were true during its last evaluation
};
