// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Utilities.h"
#include "UObject/Object.h"
#include "CiFEffect.generated.h"

class UCiFPredicate;
class UCiFRule;

USTRUCT()
struct FEffectSaliencyValues
{
	GENERATED_BODY()

	inline static constexpr int8 EFFECT_TOO_SOON = 6;
	inline static constexpr int8 LOW_NETWORK_SALIENCE = 2;
	inline static constexpr int8 MEDIUM_NETWORK_SALIENCE = 2;
	inline static constexpr int8 HIGH_NETWORK_SALIENCE = 2;
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

	/** Scores, stores and returns the salience of this effect */
	int8 scoreSalience();

	/**
	 * Checks the effect's condition rule for a CKB predicate (which constitutes a CKB item reference)
	 * @return True if a CKB reference exists in the effect's change rule
	 */
	bool hasCKBReference() const;

	bool hasSFDBLabel() const;

	/**
	 * @return predicate of type CKBEntry or null if no such predicate in effect's condition rule
	 */
	UCiFPredicate* getCKBReferencePredicate() const;

	static UCiFEffect* loadFromJson(const TSharedPtr<FJsonObject> json);
	
public:
	IdType mId;
	IdType mInstantiationId; // ID of the instantiation this effect uses for performance realization
	IdType mRejectId; // the unique identifier of this effect's reject
	int8 mSalienceScore; // Approximate measure of how "awesome" we think this effect will be
	UCiFRule* mCondition; // condition for if this effect can be happen
	UCiFRule* mChange; // the rule containing the social change associated with the effect
	bool mIsAccept; // true if the effect is in the accept branch of the social game and false if in reject branch
	int32 mLastSeenTime; // stores what was the last CiF.Time the instantiation was last seen
	FName mReferenceAsNLG; // the english interpretation of the effect's outcome to be used when this effect is referenced in later gameplay

private:

	static UniqueIDGenerator mIDGenerator;
	
};
