// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFInfluenceRule.h"

UCiFInfluenceRule* UCiFInfluenceRule::loadFromJson(TSharedPtr<FJsonObject> ruleJson)
{
	auto ir = NewObject<UCiFInfluenceRule>();
	ir = static_cast<UCiFInfluenceRule*>(Super::loadFromJson(ruleJson, ir));

	ir->mWeight = ruleJson->GetNumberField("_weight");

	return ir;
}
