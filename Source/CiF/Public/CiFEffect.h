// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CiFGameObject.h"
#include "Utilities.h"
#include "UObject/Object.h"
#include "CiFEffect.generated.h"

class UCiFGameObject;
class UCiFPredicate;
class UCiFRule;

USTRUCT()
struct FEffectSaliencyValues
{
	GENERATED_BODY()

	inline static constexpr int8 EFFECT_TOO_SOON = 6;
	inline static constexpr int8 VERY_LOW_SALIENCE = 1;
	inline static constexpr int8 LOW_SALIENCE = 2;
	inline static constexpr int8 MEDIUM_SALIENCE = 3;
	inline static constexpr int8 HIGH_SALIENCE = 4;
	inline static constexpr int8 VERY_HIGH_SALIENCE = 5;
	inline static constexpr int8 UNRECOGNIZED_NETWORK_SALIENCE = 2;
};

/**
 * 
 */
UCLASS()
class CIF_API UCiFEffect : public UObject
{
	GENERATED_BODY()

public:
	UCiFEffect();

	/**
	 * Stores and returns the salience of this effect.
	 * The saliency used when we have several effects for a social exchange - then we
	 * need to choose which effect is the most suitable to be played out.
	 * For now, the predicates in the condition rule of the effect are the ones that
	 * impacts the score the most - this because we assume that the "stricter" the condition
	 * rule (stricter is subjective by how we choose to score each type of predicate),
	 * the more "rare/special/interesting" this effect's change rule is.
	 *
	 * NOTE: the score numbers may be needed to be rebalanced if some effects are too
	 * dominant and getting chosen all the time
	 */
	int8 scoreSalience();

	/**
	 * Evaluations the condition of the Effect for truth given the current
	 * game state.
	 * 
	 * @param	initiator	The initiator of the social game.
	 * @param	responder	The responder of the social game.
	 * @param	other		A third party in the social game.
	 * @return True if all the predicates in the condition are true. Otherwise,
	 * false.
	 */
	bool evaluateCondition(UCiFGameObject* initiator, UCiFGameObject* responder=nullptr, UCiFGameObject* other=nullptr) const;

	/**
	 * Updates the social state if given the predicates in this valuation
	 * rule.
	 * 
	 * @param	initiator	The initiator of the social game.
	 * @param	responder	The responder of the social game.
	 * @param	other		A third party in the social game.
	 */
	void valuation(UCiFGameObject* initiator, UCiFGameObject* responder = nullptr, UCiFGameObject* other = nullptr) const;
	/**
	 * Checks the effect's condition rule for a CKB predicate (which constitutes a CKB item reference)
	 * @return True if a CKB reference exists in the effect's change rule
	 */
	bool hasCKBReference() const;

	bool hasSFDBLabel() const;

	bool isRoleRequired(const FName role) const;

	/**
	 * @return predicate of type CKBEntry or null if no such predicate in effect's condition rule
	 */
	UCiFPredicate* getCKBReferencePredicate() const;

	/****************************** Utility methods ***********************************/
	
	void toString(FString& outStr) const;
	static UCiFEffect* loadFromJson(const TSharedPtr<FJsonObject> json, const UObject* worldContextObject);
	
public:
	ID_t mId;
	ID_t mInstantiationId; // ID of the instantiation this effect uses for performance realization
	ID_t mRejectId; // the unique identifier of this effect's reject
	int8 mSalienceScore; // Approximate measure of how "awesome" we think this effect will be
	UPROPERTY()
	UCiFRule* mCondition; // condition for if this effect can be happen

	UPROPERTY()
	UCiFRule* mChange; // the rule containing the social change associated with the effect
	bool mIsAccept; // true if the effect is in the accept branch of the social game and false if in reject branch
	int32 mLastSeenTime; // stores what was the last CiF.Time the instantiation was last seen
	FName mReferenceAsNLG; // the english interpretation of the effect's outcome to be used when this effect is referenced in later gameplay

private:

	static UniqueIDGenerator mIDGenerator;
	
};
