// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CiFCharacter.h"
#include "Utilities.h"
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

	UCiFRule();
	
	/**
	 * @return True iff the input role is required for processing the rule
	 */
	UFUNCTION(BlueprintCallable, Category = "CiF")
	bool isRoleRequired(const FName role) const;

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
	bool evaluate(UCiFGameObject* initiator, UCiFGameObject* responder, UCiFGameObject* other = nullptr, const UCiFSocialExchange* se = nullptr);

	/**
	 * Performs valuation (aka updating the social state according to
	 * parameterized predicates) for every predicate in the rule.
	 * 
	 * @param	initiator The initiator of the social game.
	 * @param	responder	The responder of the social game.
	 * @param	other		A third party in the social game.
	 */
	void valuation(UCiFGameObject* initiator, UCiFGameObject* responder = nullptr, UCiFGameObject* other = nullptr);

	/**
	 * Any given rule should only have a single intent, the function stop searching when it finds the first intent.
	 *
	 * @return index of the intent in the predicate vector or -1 if no intent was found
	 */
	int32 findIntentIndex();

	void toString(FString& outStr);
	
	/* The additional inputRule is for the case where we load a subclass of this class.
	 * in this case, the input pointer will be filled, otherwise a new object will be filled
	 * and returned
	 */
	static UCiFRule* loadFromJson(TSharedPtr<FJsonObject> ruleJson, const UObject* worldContextObject, UCiFRule* inputRule=nullptr);
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

	/*
	 * Evaluates a rule when mGrouped=true. This is the case where the predicates aren't tested for all of them to
	 * be true. In this case, predicates are split into groups by their _group json key, where each group will have
	 * OR/AND between its group predicates, and the opposite AND/OR between the groups themselves. e.g.
	 * (A || B || C) && (D || E || F) or (A && B) || (C && D)
	 */
	bool evaluateGroupedRule(UCiFGameObject* initiator, UCiFGameObject* responder, UCiFGameObject* other, const UCiFSocialExchange* se);
public:

	FName mName;
	FString mDescription; // author description of the rule
	IdType mID; // the unique identifier of this rule - TODO not sure this is needed

	// true if this rule requires more complex evaluation.
	// for example, if initiator is male AND responder is female OR init is female AND res is male.
	// in this case we need to do OR between some groups of AND. in this case [mGroupOperator] should be "OR"
	// to indicate that we AND inside the groups and OR between the groups.
	bool mGrouped = false; 
	FName mGroupOperator = "AND";
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CiF")
	TArray<UCiFPredicate*> mPredicates; // the array of predicates that comprise this rule

private:
	static UniqueIDGenerator mIDGenerator;
	int32 mMaxSFDBOrder = 0; // stores the max SFDB order of the predicates. This filled after the first call to getHighestSFDBOrder()
};
