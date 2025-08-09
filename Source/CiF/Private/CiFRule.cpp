// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFRule.h"

#include <vector>

#include "CiFManager.h"
#include "CiFPredicate.h"
#include "CiFSubsystem.h"

UniqueIDGenerator UCiFRule::mIDGenerator = UniqueIDGenerator();

UCiFRule::UCiFRule()
{
	mID = mIDGenerator.getId();
}

bool UCiFRule::isRoleRequired(const FName role) const
{
	bool isRoleRequired = false;

	for (const auto pred : mPredicates) {
		//We can't allow ourselves to be mis-lead by predicates that are 'num times uniquely true'
		//predicates.  We ONLY want those to count as requiring a role if there is a role specified
		//in the role slot that we care about (first, second, or either first or second if 'both' is checked.
		if (pred->mIsNumTimesUniquelyTruePred) {
			switch (pred->mNumTimesRoleSlot) {
				case ENumTimesRoleSlot::INVALID:
					UE_LOG(LogTemp, Warning, TEXT("Invalid ENumTimesRoleSlot"));
					break;
				case ENumTimesRoleSlot::FIRST:
					if (pred->mPrimary == role) {
						return true;
					}
					break;
				case ENumTimesRoleSlot::SECOND:
					if (pred->mSecondary == role) {
						return true;
					}
					break;
				case ENumTimesRoleSlot::BOTH:
					if (pred->mPrimary == role || pred->mSecondary == role) {
						return true;
					}
					break;
			}
			continue; // continue to the next pred if the input role wasn't found in the role specified role slot in the pred
		}

		if (pred->mPrimary == role || pred->mSecondary == role || pred->mTertiary == role) {
			isRoleRequired = true;
		}

		if (isRoleRequired) {
			return true;
		}
	}

	return isRoleRequired;
}

bool UCiFRule::evaluate(UCiFGameObject* initiator, UCiFGameObject* responder, UCiFGameObject* other, const UCiFSocialExchange* se)
{
	// if there is a time ordering dependency in this rule
	if (getHighestSFDBOrder() > 0) {
		bool val = evaluateTimeOrderedRule(initiator, responder, other);
		return val;
	}

	
	if (mGrouped) {
		return evaluateGroupedRule(initiator, responder, other, se);
	}
	else {
		for (const auto pred : mPredicates) {
			if (!pred) {
				UE_LOG(LogTemp, Error, TEXT("this shouldn't happen but for some reason it does"));
			}
			if (!pred->evaluate(initiator, responder, other, se)) {
				return false;
			}
		}	
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
			outStr += " & ";
		}
	}
}

int32 UCiFRule::getHighestSFDBOrder()
{
	if (mMaxSFDBOrder > 0) {
		return mMaxSFDBOrder;	
	}

	int32 order = 0;
	for (const auto pred : mPredicates) {
		if (pred->mSFDBOrder > order) {
			order = pred->mSFDBOrder;
		}
	}
	mMaxSFDBOrder = order;
	return mMaxSFDBOrder;	
}

bool UCiFRule::evaluateTimeOrderedRule(UCiFGameObject* primary, UCiFGameObject* secondary, UCiFGameObject* tertiary)
{
	auto outer = GetOuter();
	if (outer) {
		UE_LOG(LogTemp, Warning, TEXT("found outer for this rule %s"), *(mName.ToString()));
	}
	auto world = GetOuter()->GetWorld();
	if (!world) {
		UE_LOG(LogTemp, Warning, TEXT("Failed getting world"));
	}
	const auto cifManager = world->GetGameInstance()->GetSubsystem<UCiFSubsystem>()->getInstance();

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

bool UCiFRule::evaluateGroupedRule(UCiFGameObject* initiator,
	UCiFGameObject* responder,
	UCiFGameObject* other,
	const UCiFSocialExchange* se)
{
	int currentGroup = -1;
	std::vector<bool> groups;

	// go over the predicates and aggregate their evaluations based on the group operator.
	// NOTE: here assuming that the predicates of the same group are consecutive
	for (const auto pred : mPredicates) {
		auto g = pred->mGroup;
		if (g != currentGroup) {
			currentGroup = g;
			groups.push_back(mGroupOperator != "AND");
		}

		bool eval = pred->evaluate(initiator, responder, other, se);
		groups[groups.size() - 1] = (mGroupOperator == "AND") ? (groups[groups.size() - 1] || eval) : (groups[groups.size() - 1] && eval);
	}

	// if operator between groups is AND, go over the groups and return on the first false encountered, otherwise return true
	if (mGroupOperator == "AND") {
		for (const auto e : groups) {
			if (!e) return false; 
		}
		return true;
	}

	// if reached here the operator is OR, go over the elements and if encounter true, return true, else return false
	for (const auto e : groups) {
		if (e) return true;
	}
	return false;
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
	
	FString name = "part of a condition/change rule";
	ruleJson->TryGetStringField(TEXT("_name"), name);
	localRule->mName = FName(name);
	
	localRule->mDescription = "";
	ruleJson->TryGetStringField(TEXT("_description"), localRule->mDescription);

	FString op;
	ruleJson->TryGetStringField(TEXT("_operator"), op);
	localRule->mGroupOperator = FName(op);

	ruleJson->TryGetBoolField(TEXT("_grouped"), localRule->mGrouped);
	
	// load predicate
	auto predicateJson = ruleJson->GetArrayField(TEXT("Predicate"));
	for (const auto predJson : predicateJson) {
		auto predicate = UCiFPredicate::loadFromJson(predJson->AsObject(), worldContextObject);
		localRule->mPredicates.Add(predicate);
	}

	return localRule;
}
