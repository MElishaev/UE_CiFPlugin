// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFMicrotheory.h"
#include "CiFCast.h"
#include "CiFInfluenceRule.h"
#include "CiFInfluenceRuleSet.h"
#include "CiFManager.h"
#include "CiFRule.h"
#include "CiFSubsystem.h"
#include "Kismet/GameplayStatics.h"

UCiFMicrotheory::UCiFMicrotheory()
{
	mInitiatorIR = NewObject<UCiFInfluenceRuleSet>();
	mResponderIR = NewObject<UCiFInfluenceRuleSet>();
	mDefinition = nullptr; // will be loaded by the loadFromJson
}

float UCiFMicrotheory::score(UCiFCharacter* initiator,
                             UCiFGameObject* responder,
                             UCiFSocialExchange* se,
                             const TArray<UCiFGameObject*>& others) const
{
	const auto cifManager = GetWorld()->GetGameInstance()->GetSubsystem<UCiFSubsystem>()->getInstance();
	const TArray<UCiFGameObject*> possibleOthers = others.Num() > 0 ? others : static_cast<TArray<UCiFGameObject*>>(cifManager->mCast->mCharacters);
	float totalScore = 0;

	if (mDefinition->isRoleRequired("other")) {
		// TODO: there is no micro-theory definition that requires other or any IR inside a MT that requires it... can be deleted
		// if the definition is about an other, if it is true for even one other, run the micro-theory
		for (const auto other : possibleOthers) {
			if ((other->mObjectName != initiator->mObjectName) && (other->mObjectName != responder->mObjectName)) {
				if (mDefinition->evaluate(initiator, responder, other, se)) {
					// do not reverse roles here, because whichever IRS we are using, the roles are how they ought to be
					// role reversal for MTs happens at parsing xml time.
					totalScore += mInitiatorIR->scoreRules(initiator, responder, other, se, mName);
				}
			}
		}
	}
	else {
		if (mDefinition->evaluate(initiator, responder, nullptr, se)) {
			// if the definition of the MT holds with the current participants, score the IR
			totalScore += mInitiatorIR->scoreRulesWithVariableOther(initiator,
			                                                        responder,
			                                                        nullptr,
			                                                        se,
			                                                        possibleOthers,
			                                                        mName);
		}
	}

	return totalScore;
}

UCiFMicrotheory* UCiFMicrotheory::loadFromJson(TSharedPtr<FJsonObject> json, const UObject* worldContextObject)
{
	auto mt = NewObject<UCiFMicrotheory>(const_cast<UObject*>(worldContextObject));

	mt->mName = FName(json->GetStringField("Name"));
	UE_LOG(LogTemp, Log, TEXT("Parsing microtheory: %s"), *(mt->mName.ToString()))

	const auto definitionJson = json->GetObjectField("Definition");
	mt->mDefinition = UCiFRule::loadFromJson(definitionJson, worldContextObject);
	
	auto irsJson = json->GetArrayField("InitiatorInfluenceRuleSet");
	for (const auto irJson : irsJson) {
		const auto ir = UCiFInfluenceRule::loadFromJson(irJson->AsObject(), worldContextObject);
		mt->mInitiatorIR->mInfluenceRules.Add(ir);
	}

	auto rirsJson = json->GetArrayField("ResponderInfluenceRuleSet");
	for (const auto irJson : rirsJson) {
		const auto ir = UCiFInfluenceRule::loadFromJson(irJson->AsObject(), worldContextObject);
		mt->mResponderIR->mInfluenceRules.Add(ir);
	}
	
	return mt;
}
