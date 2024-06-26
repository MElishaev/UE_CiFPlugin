// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CiFGameObject.h"
#include "CiFPredicate.h"
#include "CiFSFDBContext.h"
#include "Utilities.h"
#include "CiFTriggerContext.generated.h"

class UCiFRule;
struct FSFDBLabel;
class UCiFCharacter;
enum class ESFDBLabelType : uint8;
/**
 * 
 */
UCLASS()
class CIF_API UCiFTriggerContext : public UCiFSFDBContext
{
	GENERATED_BODY()

public:
	virtual ESFDBContextType getType() const override;

	/**
	 * Determines if the SocialGameContext represents a status change consistent
	 * with the passed-in Predicate.
	 * 
	 * @param	pred	Predicate to check for.
	 * @param	x	Primary character.
	 * @param	y	Secondary character.
	 * @param	z	Tertiary character.
	 * @return	True if the SocialGameContext's change is the same as the valuation
	 * of p. False if not.
	 */
	virtual bool isPredicateInChange(const UCiFPredicate* pred,
	                                 const UCiFGameObject* x,
	                                 const UCiFGameObject* y,
	                                 const UCiFGameObject* z) override;

	/**
	 * This one is used with numTimesUniquelyTrue sfdb label predicates
	 * 
	 * Determines if the passed-in parameters of label and characters match the SFDBLabel and
	 * characters related to the label in this SocialGameContext. If this context is a backstory
	 * context, the first and second character parameters must match the context's initiator and
	 * responder respectively. If it is a non-backstory context, the first and second character
	 * parameters must match the labelArg1 and labelArg2 properties respectively.
	 * 
	 * @param	labelType	The label to match. If WILDCARD is passed in, all labels are considered to match.
	 * @param	first		The first character parameter.
	 * @param	second		The second character parameter.
	 * @param	third		The third character parameter (not currently used).
	 * @return	True if this context is a match to the parameters. False if not a match.
	 */
	bool doesSFDBLabelMatchStrict(const ESFDBLabelType labelType,
	                              const UCiFGameObject* first,
	                              const UCiFGameObject* second = nullptr,
	                              const UCiFGameObject* third = nullptr,
	                              const UCiFPredicate* pred = nullptr) const;

	/**
	 * Determines if the passed-in parameters of label and characters match the SFDBLabel and
	 * characters related to the label in this SocialGameContext. If this context is a backstory
	 * context, the first and second character parameters must match the context's initiator and
	 * responder respectively. If it is a non-backstory context, the first and second character
	 * parameters must match the labelArg1 and labelArg2 properties respectively.
	 * 
	 * @param	labelType	The label to match. If -1 is passed in, all labels are considered to match.
	 * @param	first		The first character parameter.
	 * @param	second		The second character parameter.
	 * @param	third		The third character parameter (not currently used).
	 * @return	True if this context is a match to the parameters. False if not a match.
	 */
	bool doesSFDBLabelMatch(const ESFDBLabelType labelType,
	                        const UCiFGameObject* first,
	                        const UCiFGameObject* second = nullptr,
	                        const UCiFGameObject* third = nullptr,
	                        const UCiFPredicate* pred = nullptr) const;

	UCiFRule* getChange() const;

private:
	/**
	 * Determines if the character names in context's predicates primary, secondary, 
	 * and tertiary character variables match the character names from the non-context
	 * character variables x,y, and z respectively.
	 * 
	 * @param	predInChange	A predicate in the context's change.
	 * @param	c1				Primary character of non-context predicate.
	 * @param	c2				secondary character of non-context predicate.
	 * @param	c3				Tertiary character of non-context predicate.
	 * @return	True if the character names in context's predicates primary, secondary, 
	 * and tertiary character variables match the character names from the non-context
	 * character variables x,y, and z respectively.
	 */
	bool doPredicateRoleMatchCharacterVariables(UCiFPredicate* predInChange,
	                                            UCiFGameObject* c1,
	                                            UCiFGameObject* c2,
	                                            UCiFGameObject* c3,
	                                            UCiFPredicate* predInEvalRule = nullptr) const;

	/**
	 * Determines if the character variable binding between the context and the predicate's
	 * primary character reference match. This is not bidirectional.
	 * 
	 * @param	predInEvalRule	A predicate in the context's change.
	 * @param	x				Primary character of non-context predicate.
	 * @param	y				secondary character of non-context predicate.
	 * @param	z				Tertiary character of non-context predicate.
	 * @return	True if the character names in context's predicates primary
	 * character variable matches the character names from the non-context
	 * character variables, x,y, and z.
	 */
	bool doesPredicateRoleMatch(UCiFPredicate* predInEvalRule,
	                            const FName roleInEval,
	                            UCiFPredicate* predInChange,
	                            const FName roleInChange,
	                            UCiFGameObject* x,
	                            UCiFGameObject* y,
	                            UCiFGameObject* z) const;

public:
	FName mInitiatorName; // name of the initiator of this social exchange
	FName mResponderName; // name of the responder of this social exchange
	FName mOther;         // name of the other of this social exchange

	IdType mId;                     // TODO - maybe this needs to be FName, or this needs to be in the base class
	UCiFRule* mStatusTimeoutChange; // is this a trigger that occurs from a status ending?

	// todo - why is there many labels associated with this context entry? i need an example to understand what does it mean
	TArray<FSFDBLabel> mSFDBLabels; // the SFDB labels associated with this context entry 
};
