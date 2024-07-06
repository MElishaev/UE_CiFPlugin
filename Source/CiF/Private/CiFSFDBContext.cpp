// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFSFDBContext.h"

ESFDBContextType UCiFSFDBContext::getType() const
{
	// this is default implementation - we shouldn't reach here (UE won't let me leave it without implementation)
	return ESFDBContextType::INVALID;
}

bool UCiFSFDBContext::isPredicateInChange(const UCiFPredicate* pred,
	const UCiFGameObject* x,
	const UCiFGameObject* y,
	const UCiFGameObject* z)
{
	// this is default implementation - we shouldn't reach here (UE won't let me leave it without implementation)
	return false;	
}

void UCiFSFDBContext::loadFromJson(const TSharedPtr<FJsonObject> json, const UObject* worldContextObject)
{
	mTime = json->GetNumberField("_time");
}

auto UCiFSFDBContext::operator<(const UCiFSFDBContext& o) const
{
	return mTime < o.mTime;
}
