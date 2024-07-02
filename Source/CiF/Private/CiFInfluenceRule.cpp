// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFInfluenceRule.h"

UCiFInfluenceRule* UCiFInfluenceRule::loadFromJson(TSharedPtr<FJsonObject> ruleJson, const UObject* worldContextObject)
{
	auto ir = NewObject<UCiFInfluenceRule>(const_cast<UObject*>(worldContextObject));
	ir = static_cast<UCiFInfluenceRule*>(Super::loadFromJson(ruleJson, worldContextObject, ir));

	ir->mWeight = ruleJson->GetNumberField("_weight");

	return ir;
}
