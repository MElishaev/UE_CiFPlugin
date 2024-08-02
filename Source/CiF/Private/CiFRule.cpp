// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFRule.h"

#include "CiFManager.h"
#include "CiFPredicate.h"
#include "CiFSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Json.h"

UniqueIDGenerator UCiFRule::mIDGenerator = UniqueIDGenerator();

UCiFRule::UCiFRule()
{
	mID = mIDGenerator.getId();
}

bool UCiFRule::isThirdCharacterRequired()
{
	bool isThirdCharRequired = false;

	for (const auto pred : mPredicates) {
		//We can't allow ourselves to be mis-lead by predicates that are 'num times uniquely true'
		//predicates.  We ONLY want those to count as requiring an other if there is an other specified
		//in the role slot that we care about (first, second, or either first or second if 'both' is checked.
		if (pred->mIsNumTimesUniquelyTruePred) {
			switch (pred->mNumTimesRoleSlot) {
				case ENumTimesRoleSlot::INVALID:
					UE_LOG(LogTemp, Warning, TEXT("Invalid ENumTimesRoleSlot"));
					break;
				case ENumTimesRoleSlot::FIRST:
					//if the first role is an 'other', then return true.  Otherwise, we can move on to the next predicate.
					if (pred->mPrimary == "other") {
						return true;
					}
					break;
				case ENumTimesRoleSlot::SECOND:
					//if the second role is an 'other', then return true.  Otherwise, we can move on to the next predicate.
					if (pred->mSecondary == "other") {
						return true;
					}
					break;
				case ENumTimesRoleSlot::BOTH:
					//if either the first or second role is an 'other', then return true.  Otherwise we move on to next predicate.
					if (pred->mPrimary == "other" || pred->mSecondary == "other") {
						return true;
					}
					break;
			}
		}

		if (pred->mPrimary == "other" || pred->mSecondary == "other" || pred->mTertiary == "other") {
			isThirdCharRequired = true;
		}

		if (isThirdCharRequired) {
			return true;
		}
	}

	return isThirdCharRequired;
}

bool UCiFRule::evaluate(UCiFGameObject* initiator, UCiFGameObject* responder, UCiFGameObject* other, UCiFSocialExchange* se)
{
	mLastTrueCount = 0;

	//if there is a time ordering dependency in this rule, use the evaluateTimeOrderedRule() pipeline.
	if (getHighestSFDBOrder() > 0) {
		return evaluateTimeOrderedRule(initiator, responder, other);
	}

	for (const auto pred : mPredicates) {
		if (!pred->evaluate(initiator, responder, other, se)) {
			return false;
		}
		mLastTrueCount++;
	}
	
	return true;
}

void UCiFRule::valuation(UCiFGameObject* initiator, UCiFGameObject* responder, UCiFGameObject* other)
{
	for (auto p : mPredicates) {
		p->valuation(initiator, responder, other);
	}
}

int32 UCiFRule::findIntentIndex()
{
	for (int32 i = 0; i < mPredicates.Num(); i++) {
		if (mPredicates[i]->mIsIntent) {
			return i;
		}
	}
	return -1;
}

void UCiFRule::toString(FString& outStr)
{
	for (int i = 0; i < mPredicates.Num(); i++) {
		FString predStr;
		mPredicates[i]->toString(predStr);
		outStr += predStr;
		if (i < mPredicates.Num() - 1) {
			outStr += " ^ ";
		}
	}
}

int32 UCiFRule::getHighestSFDBOrder()
{
	int32 order = 0;
	for (const auto pred : mPredicates) {
		if (pred->mSFDBOrder > order) {
			order = pred->mSFDBOrder;
		}
	}
	return order;
}

bool UCiFRule::evaluateTimeOrderedRule(UCiFGameObject* primary, UCiFGameObject* secondary, UCiFGameObject* tertiary)
{
	const auto cifManager = GetWorld()->GetGameInstance()->GetSubsystem<UCiFSubsystem>()->getInstance();

	const auto maxOrderInRule = getHighestSFDBOrder(); // max order value of the rule

	//when evaluating an order, this value is updated with the highest truth time for the order.
	auto curOrderTruthTime = cifManager->mSFDB->getLowestContextTime();

	//the highest truth time of all the predicates in the previous order.
	auto lastOrderTruthTime = curOrderTruthTime;

	for (int order = 1; order < maxOrderInRule; order++) {
		for (const auto pred : mPredicates) {
			if (pred->mSFDBOrder == order) {
				//the predicate is of the order we are currently concerned with
				const auto time = cifManager->mSFDB->timeOfPredicateInHistory(pred, primary, secondary, tertiary);

				//was the predicate true at all in history? If not, return false.
				if (time == UCiFSocialFactsDataBase::INVALID_TIME) {
					return false;
				}

				//this predicate was true only before the last order, so rule is not true.
				if (time < lastOrderTruthTime) {
					return false;
				}

				//update curOrderTruthTime to highest value for this order
				if (time > curOrderTruthTime) {
					curOrderTruthTime = time;
				}

				//if the preceding conditions are passed, this predicate of the rule is true; continue to next predicate.
			}
		}
		lastOrderTruthTime = curOrderTruthTime;
	}

	//evaluate the predicates in the rule that are not time sensitive (i.e. their order is less than 1).
	for (const auto pred : mPredicates) {
		if (pred->mSFDBOrder < 1) {
			if (!pred->evaluate(primary, secondary, tertiary)) {
				return false;
			}
		}
	}

	return true;
}

UCiFRule* UCiFRule::loadFromJson(TSharedPtr<FJsonObject> ruleJson, const UObject* worldContextObject, UCiFRule* inputRule)
{
	
	auto localRule = inputRule ? inputRule : NewObject<UCiFRule>(const_cast<UObject*>(worldContextObject));

	// handle the case where the rule is empty - this can happen when there is no condition rule in an effect
	if (ruleJson->Values.IsEmpty()) {
		localRule->mDescription = "empty rule";
		localRule->mPredicates.Empty();
		return localRule;
	}
	
	FString name;
	if (!ruleJson->TryGetStringField("_name", name)) {
		localRule->mName = "part of a condition/change rule";
	}
	localRule->mName = FName(name);
	
	localRule->mDescription = "";
	ruleJson->TryGetStringField("_description", localRule->mDescription);
	
	// load predicate
	auto predicateJson = ruleJson->GetArrayField("Predicate");
	for (const auto predJson : predicateJson) {
		auto predicate = UCiFPredicate::loadFromJson(predJson->AsObject(), worldContextObject);
		localRule->mPredicates.Add(predicate);
	}

	return localRule;
}
