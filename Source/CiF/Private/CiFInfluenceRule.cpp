// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFInfluenceRule.h"
#include "CiFPredicate.h"

void UCiFInfluenceRule::toNLG(FString& outStr, const FName initiatorName, const FName responderName, const FName otherName)
{
	for (auto p : mPredicates) {
		outStr += p->toNLG(initiatorName, responderName, otherName);
	}
}

UCiFInfluenceRule* UCiFInfluenceRule::loadFromJson(TSharedPtr<FJsonObject> ruleJson, const UObject* worldContextObject)
{
	auto ir = NewObject<UCiFInfluenceRule>(const_cast<UObject*>(worldContextObject));
	ir = static_cast<UCiFInfluenceRule*>(Super::loadFromJson(ruleJson, worldContextObject, ir));

	ir->mWeight = ruleJson->GetNumberField("_weight");

	return ir;
}
