// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFMicrotheory.h"
#include "CiFCast.h"
#include "CiFInfluenceRuleSet.h"
#include "CiFManager.h"
#include "CiFRule.h"
#include "CiFSubsystem.h"
#include "Kismet/GameplayStatics.h"

float UCiFMicrotheory::score(UCiFCharacter* initiator,
                             UCiFCharacter* responder,
                             UCiFSocialExchange* se,
                             TArray<UCiFCharacter*> others) const
{
	auto cifManager = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UCiFSubsystem>()->getInstance();
	const auto& possibleOthers = others.Num() > 0 ? others : cifManager->mCast->mCharacters;
	float totalScore = 0;

	if (mDefinition->isOtherCharacterRequired()) {
		// if the definition is about an other, if it is true for even one other, run the microtheory
		for (const auto other : possibleOthers) {
			if ((other->mObjectName != initiator->mObjectName) && (other->mObjectName != responder->mObjectName)) {
				if (mDefinition->evaluate(initiator, responder, other, se)) {
					// do not reverse roles here, because whichever IRS we are using, the roles are how they ought to be
					// role reversal for MTs happens at parsing xml time.
					totalScore += mInitiatorIR->scoreRules(initiator, responder, other, se, TArray<UCiFGameObject*>(possibleOthers), mName);
				}
			}
		}
	}
	else {
		if (mDefinition->evaluate(initiator, responder, nullptr, se)) {
			//if the definition doesn't involve an other, we score the rules with a variable other
			totalScore += mInitiatorIR->scoreRulesWithVariableOther(initiator,
			                                                        responder,
			                                                        nullptr,
			                                                        se,
			                                                        TArray<UCiFGameObject*>(possibleOthers),
			                                                        mName);
		}
	}

	return totalScore;
}
