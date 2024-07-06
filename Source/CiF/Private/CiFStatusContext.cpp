// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFStatusContext.h"

#include "CiFPredicate.h"

ESFDBContextType UCiFStatusContext::getType() const
{
	return ESFDBContextType::STATUS;
}

bool UCiFStatusContext::isPredicateInChange(const UCiFPredicate* pred,
                                            const UCiFGameObject* x,
                                            const UCiFGameObject* y,
                                            const UCiFGameObject* z)
{
	if (pred->mType != EPredicateType::STATUS) return false;
	if (pred->mIsNegated != mPredicate->mIsNegated) return false;
	if (x->mObjectName != pred->mPrimary) return false;
	if (y)
		if (y->mObjectName != pred->mSecondary) return false;
	if (z)
		if (z->mObjectName != pred->mTertiary) return false;
	return true;
}

UCiFStatusContext* UCiFStatusContext::loadFromJson(const TSharedPtr<FJsonObject> json, const UObject* worldContextObject)
{
	const auto sc = NewObject<UCiFStatusContext>(const_cast<UObject*>(worldContextObject));

	sc->UCiFSFDBContext::loadFromJson(json, worldContextObject);

	const auto predsJson = json->GetObjectField("Predicate");
	sc->mPredicate = UCiFPredicate::loadFromJson(predsJson, worldContextObject);

	return sc;
}
