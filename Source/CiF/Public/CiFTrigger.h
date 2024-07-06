// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CiFEffect.h"
#include "CiFTrigger.generated.h"

class UCiFGameObject;
class UCiFTriggerContext;
/**
 * The Trigger class consists of conditional rules that look over the recent
 * social state and perform social change based on evaluation of the conditions.
 * This allows for social state transformations that are difficult to capture
 * in SocialGame effect changes. For example, the detection of a the cheating
 * status, or where a character is dating more than one other character
 * simultaneously, is a very conditional and difficult statement in the 
 * context of social game effect changes. Furthermore, it would have to be
 * present in every effect that has dating in it's change rule. Triggers
 * centralize this logic and allow for more "third party" reasoning. An
 * example would be the emnity rule:
 * <pre>friends(x,y)^negativeAct(z,y)->emnity(x,z)</pre>
 * 
 * Affected triggers are placed into the social facts database as a
 * TriggerContext.
 * 
 * As triggers behave generally like SocialGame Effects in that they 
 * have a condition rule, a change rule, and a performance realization
 * description, they extend Effect as to not duplicate functionality.
 * 
 * Unlike the logical structures in social games, the logical rules
 * in triggers are not associated with roles; they are standard logical
 * variables -- x, y, and z. The condition and change rules of triggers
 * should use x,y, and z instead of responder, initiator, and other.
 */
UCLASS()
class CIF_API UCiFTrigger : public UCiFEffect
{
	GENERATED_BODY()

public:
	/**
	 * Returns a SFDBContext interface compatible SFDB entry for this
	 * trigger. No time is set as this function is meant to be called
	 * in either CiFSingleton or SocialFactsDB when placing a trigger
	 * that has fired into the SFDB context.
	 * @param	time The SFDB time with which to tag the SFDB entry.
	 * @return	The SFDB entry for the Trigger.
	 */
	UCiFTriggerContext* makeTriggerContext(const int32 time, UCiFGameObject* x, UCiFGameObject* y = nullptr, UCiFGameObject* z = nullptr) const;

	static UCiFTrigger* loadFromJson(const TSharedPtr<FJsonObject> json, const UObject* worldContextObject);
	
public:
	/**
	 * This is used to help know when we are dealing with an actual authored trigger, or a trigger context which has
	 * no condition, and is the result of the status timing out.
	 */
	static IdType mStatusTimeoutTriggerID;
};
