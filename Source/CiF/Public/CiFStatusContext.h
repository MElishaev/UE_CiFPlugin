// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CiFSFDBContext.h"
#include "CiFStatusContext.generated.h"

/**
 * 
 */
UCLASS()
class CIF_API UCiFStatusContext : public UCiFSFDBContext
{
	GENERATED_BODY()

public:
	virtual ESFDBContextType getType() const override;

	/**
	 * Determines if the StatusContext represents a status change consistent
	 * with the passed-in Predicate.
	 * 
	 * @param	pred	Predicate to check for.
	 * @param	x		Primary character.
	 * @param	y		Secondary character.
	 * @param	z		Tertiary character.
	 * @return	True if the StatusContext's change is the same as the valuation
	 * of p. False if not.
	 */
	virtual bool isPredicateInChange(const UCiFPredicate* pred,
	                                 const UCiFGameObject* x,
	                                 const UCiFGameObject* y,
	                                 const UCiFGameObject* z) override;

	static UCiFStatusContext* loadFromJson(const TSharedPtr<FJsonObject> json, const UObject* worldContextObject);

public:
	UCiFPredicate* mPredicate;
};
