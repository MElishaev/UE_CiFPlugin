// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CiFSocialFactsDataBase.h"
#include "CiFPredicate.generated.h"

enum class ETrait : uint8;
enum class ERelationshipType : uint8;
enum class ESocialNetworkType : uint8;
enum class EStatus : uint8;
class UCiFGameObject;
class UCiFSocialExchange;

UENUM(BlueprintType)
enum class EPredicateType : uint8
{
	INVALID UMETA(DisplayName="Invalid"),
	TRAIT UMETA(DisplayName="Trait"),
	NETWORK UMETA(DisplayName="Network"),
	RELATIONSHIP UMETA(DisplayName="Relationship"),
	///< intent predicate type (intent of the character to modify the social state)
	STATUS UMETA(DisplayName="Status"),
	CKBENTRY UMETA(DisplayName="CBKEntry"),
	SFDBLABEL UMETA(DisplayName="SFDBLabel"),
	SIZE
};

UENUM(BlueprintType)
enum class EIntentType : uint8
{
	INVALID,
	BUDDY_UP = 0,
	BUDDY_DOWN,
	ROMANCE_UP,
	ROMANCE_DOWN,
	COOL_UP,
	COOL_DOWN,
	FRIENDS,
	END_FRIENDS,
	DATING,
	END_DATING,
	ENEMIES,
	END_ENEMIES,
	SIZE
};

/* Network comparision operators (a.k.a. comparators) */
UENUM(BlueprintType)
enum class EComparatorType : uint8
{
	LESS_THAN UMETA(DisplayName="LESSTHAN"),
	GREATER_THAN UMETA(DisplayName="GREATERTHAN"),
	AVERAGE_OPINION UMETA(DisplayName="AVERAGEOPINION"),
	FRIENDS_OPINION UMETA(DisplayName="FRIENDSOPINION"),
	DATING_OPINION UMETA(DisplayName="DATINGOPINION"),
	ENEMIES_OPINION UMETA(DisplayName="ENEMIESOPINION"),
	SIZE
};

/* Network comparision operators (a.k.a. comparators) */
UENUM(BlueprintType)
enum class EOperatorType : uint8
{
	INCREASE,
	///< for increasing network value
};

// todo what is that?
UENUM(BlueprintType)
enum class ENumTimesRoleSlot : uint8
{
	INVALID,
	FIRST,
	SECOND,
	BOTH,
	SIZE
};

/**
 * The Predicate class is the terminal and functional end of the logic
 * constructs in CiF. All rules, influence rules, rule sets, and social
 * changes are composed of predicates.
 * 
 * Predicates have two major functions: evaluation (to determine truth
 * given the current social state) and valuation (to modify the current
 * social state). Each type of predicate has its own valuation and evaluation
 * functions to enact the logical statement within CiFs data structures.
 *
 * predicates can represent specific characters, or general roles (initiator, responder, other).
 * For example, we can create a predicate that checks if John's friendship value towards Michael
 * is greater than X, and this can represent some quest predicate.
 * Or the predicate can represent a check if initiator's some network value towards the responder
 * is greater than some X, which is a more general predicate that can be reused for different characters.
 */
UCLASS()
class CIF_API UCiFPredicate : public UObject
{
	GENERATED_BODY()

public:
	UCiFPredicate();

	/**
	 * Evaluates the predicate for truth given the characters involved bound to the parameters.
	 * The process of evaluating truth depends on the type of specific instance of predicate.
	 * @return True iff the predicate evaluates to true
	 */
	bool evaluate(const UCiFGameObject* c1,
	              const UCiFGameObject* c2 = nullptr,
	              const UCiFGameObject* c3 = nullptr,
	              const UCiFSocialExchange* se = nullptr);

	/**
	 * Evaluates the predicate for truth given the characters involved
	 * bound to the parameters and determines how many times the predicate is
	 * uniquely true. The process of evaluating truth depends
	 * on the type of the specific instance of the predicate and the number of times
	 * the predicate is uniquely true.
	 * 
	 * @param	c1 Character variable of the first predicate parameter.
	 * @param	c2 Character variable of the second predicate parameter.
	 * @param	c3 Character variable of the third predicate parameter.
	 * @return True of the predicate evaluates to true. False if it does not.
	 */
	bool evalForNumberUniquelyTrue(const UCiFGameObject* c1,
	                               const UCiFGameObject* c2,
	                               const UCiFGameObject* c3,
	                               const UCiFSocialExchange* se);

	/**
	 * Same as EvalCBKEntry but here we actually produce a list of all objects
	 * that fulfill the requirements, and returns that list of objects as an array
	 *
	 * @param first The first character who holds an opinion on this CKB object
	 * @param second same
	 * @param outArray Output array
	 */ // TODO-- all of this idea of a method is weird how you match 2 characters and how the ckbentry includes
	//			opinion of 2 characters on 1 item, and what the truth label of that item, WTF?
	void evalCKBEntryForObjects(const UCiFGameObject* first, const UCiFGameObject* second, TArray<FName>& outArray) const;

	/**
	 * Returns true if the input object to this method has the trait held by this predicate instance 
	 */
	bool evalTrait(const UCiFGameObject* first) const;

	/**
	 * Only characters should eval networks
	 */
	bool evalNetwork(const UCiFGameObject* first, const UCiFGameObject* second) const;

	/** Returns true if the character in the first parameter has the relationship
	 * noted in the status field of this class.
	 * 
	 * @param first The character for which the existence of the status is ascertained.
	 * @return True if the character has the status. False if the status is not present.
	 */
	bool evalStatus(const UCiFGameObject* first, const UCiFGameObject* second) const;

	bool evalCKBEntry(const UCiFGameObject* first, const UCiFGameObject* second) const;

	/** Returns true if the character in the first parameter has the relationship
	 * noted in the relationship type field of this class.
	 * 
	 * @return True if the noted relationship occurs between the input characters
	 */
	bool evalRelationship(const UCiFGameObject* first, const UCiFGameObject* second) const;

	/**
	 * Evaluates the truth of the SFDBLabel type Predicate given a set of characters. This
	 * evaluation is different from most other predicates in that it is always based in 
	 * history. Every SFDBLabel type predicate looks back in the social facts database to
	 * see if the label of the predicate was true for the characters in the past. If there
	 * is a match in history, it is evaluated true.
	 * 
	 * Note: we currently do not allow for a third character to be used.
	 */
	bool evalSFDBLabel(const UCiFGameObject* first, const UCiFGameObject* second, const UCiFGameObject* third) const;

	/**
     * Determines if the structures of two Predicates are equal where
     * structure is every Predicate parameter other than the character 
     * variables (primary, secondary, and tertiary) and isSFDB.
     * 
     * This function is for comparing ordered rules (which represent change) 
     * with effect change rules in SGs played in the past.
     * 
     * @return	True if the structures of the Predicate match or false if they do not.
     */
	static bool equalsValuationStructure(const UCiFPredicate* p1, const UCiFPredicate* p2);

	/* if it is an intent, return the intent ID, other wise returns -1 // TODO-modify it because we wont with ENUMS */
	EIntentType getIntentType();

	/**
	 * Determines the value class of the primary property and returns a
	 * value based on that class. If it is a role class, "initiator", 
	 * "responder" or "other" will be returned. If it is a variable class,
	 * "x","y" or "z" will be returned. If it is anything else (character
	 * name or a mispelling), the raw value will be returned.
	 * @return The value based on the value class of the primary property.
	 */
	FName getRoleValue(const FName val) const;

	/**
	 * Determines the character name bound to the second (secondary) character variable role in this predicate given
	 * a context of characters in roles.
	 * @param	initiator
	 * @param	responder
	 * @param	other
	 * @return The name of the secondary character associated with the predicate character variable and role context.
	 */
	FName getSecondaryCharacterNameFromVariables(const UCiFGameObject* initiator,
	                                             const UCiFGameObject* responder,
	                                             const UCiFGameObject* other) const;
	FName getPrimaryCharacterNameFromVariables(const UCiFGameObject* initiator,
	                                           const UCiFGameObject* responder,
	                                           const UCiFGameObject* other) const;

private:
	FName getValueOfPredicateVariable(const FName var) const;

	ERelationshipType comparatorTypeToRelationshipType(const EComparatorType comparatorType) const;

	/* returns true if this predicate represent a SFDB predicate with a category label */
	bool isSFDBLabelCategory() const;

	/* clears the member variables to default state */
	void clear();

public:
	/* The character variable to which this predicate applies.
	 * example for a predicate to give an idea how primary/secondary are related to first and second in
	 * the evaluate() method:
	 * <Predicate type="network" networkType="trust" first="responder" second="initiator" comparator="-"
	 *	value="0" negated="false" name="intent-network down: trust (responder toward initiator)"
	 *	intent="true" intentType="1" isSFDB="false" window="0" numTimesUniquelyTrueFlag="false"
	 *	numTimesUniquelyTrue="0" numTimesRoleSlot=""/>
	 */
	UPROPERTY()
	FName mPrimary;

	UPROPERTY()
	FName mSecondary;

	UPROPERTY()
	FName mTertiary;

	//TODO --	is this really the best way to implement this class? won't is be better just to
	//			create an hierarchy of subclasses which will make this class less monolithic
	UPROPERTY()
	bool mIsSFDB; // True if this predicate is an SFDB lookup

	bool mIsNegated; // True if the truth value of the predicate should be negated.

	UPROPERTY()
	EPredicateType mType;
	ETrait mTrait; // TODO - for a TRAIT predicate
	UPROPERTY()
	int32 mWindowSize; // how long to look back in SFDB // TODO - this is also SFDB only param
	FSFDBLabel mSFDBLabel;

	int32 mSFDBOrder; // TODO - what is this?

	bool mIsIntent; // true if this predicate is intent predicate TODO - this is specifically for an intent predicates

	EStatus mStatusType; // todo- another member relevant only for status predicates

	EComparatorType mComparatorType; // TODO - this is relevant only for network predicates
	EOperatorType mOperatorType; // TODO - this is relevant only for network predicates
	ESocialNetworkType mNetworkType; // TODO - this is relevant only for network predicates
	ERelationshipType mRelationshipType; // TODO - this is relevant only for network predicates
	uint8 mNetworkValue; // network value for comparisons if this is a network predicate TODO - this is relevant only for network predicates

	bool mIsNumTimesUniquelyTruePred; // Flag that specifies if this is a "number of times this pred is uniquely true" type pred
	uint16 mNumTimesUniquelyTrue;     // the number of times this predicate needs to be uniquely true
	ENumTimesRoleSlot mNumTimesRoleSlot;
	// todo - what is this shit? my assumption for now is that it indicates which role the predicate goes towards? or from?

	FName mFirstSubjectiveLink;  // todo - what is the purpose of this?
	FName mSecondSubjectiveLink; // todo - what is the purpose of this?
	FName mTruthLabel;           // todo - what is the purpose of this?
};
