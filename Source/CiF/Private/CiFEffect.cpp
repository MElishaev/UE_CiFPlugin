// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFEffect.h"

#include "Engine/World.h"
#include "CiFManager.h"
#include "CiFPredicate.h"
#include "CiFRule.h"
#include "CiFSocialNetwork.h"
#include "CiFSubsystem.h"

UniqueIDGenerator UCiFEffect::mIDGenerator = UniqueIDGenerator();

UCiFEffect::UCiFEffect()
{
	mId = mIDGenerator.getId();
}

int8 UCiFEffect::scoreSalience()
{
	int8 salience = 0;

	// +6 is maybe to impactful
	for (const auto pred : mChange->mPredicates) {
		if (pred->mType == EPredicateType::SFDB_LABEL) {
			salience += FEffectSaliencyValues::VERY_HIGH_SALIENCE;
		}
	}

	/* for a quick explanation - if some predicate is negated, it means that
	 * the predicate doesn't hold. By itself, it is more likely that something doesn't hold
	 * and more rare for some status/trait/relationship be present, thus, if it holds
	 * it results in HIGHER score contribution
	 * */
	for (const auto pred : mCondition->mPredicates) {
		switch (pred->mType) {
			case EPredicateType::INVALID:
			case EPredicateType::SIZE:
				break;
			case EPredicateType::TRAIT:
				salience += pred->mIsNegated ? FEffectSaliencyValues::VERY_LOW_SALIENCE : FEffectSaliencyValues::HIGH_SALIENCE;
				break;
			case EPredicateType::NETWORK:
				switch (pred->mComparatorType) {
					/* if we are looking at a LESS_THAN comparator, then the higher the network value,
					 * the easier/more probable that the predicate will hold - hence, less saliency score.
					 * vice-versa regarding the GREATER_THAN comparator.
					 */
					case EComparatorType::LESS_THAN:
						if (pred->mNetworkValue <= 40) { // <=40 just to check if it is in the lower 1/3 of the range
							salience += FEffectSaliencyValues::MEDIUM_SALIENCE;
						}
						else if (pred->mNetworkValue <= 70) {
							salience += FEffectSaliencyValues::LOW_SALIENCE;
						}
						break;
					case EComparatorType::GREATER_THAN:
						if (pred->mNetworkValue >= 60) {
							salience += FEffectSaliencyValues::MEDIUM_SALIENCE;
						}
						else if (pred->mNetworkValue >= 30) {
							salience += FEffectSaliencyValues::LOW_SALIENCE;
						}
						break;
					default:
						salience += FEffectSaliencyValues::UNRECOGNIZED_NETWORK_SALIENCE;
				}
				break;
			case EPredicateType::RELATIONSHIP:
			case EPredicateType::STATUS:
				salience += pred->mIsNegated ? FEffectSaliencyValues::VERY_LOW_SALIENCE : FEffectSaliencyValues::MEDIUM_SALIENCE;
				break;
			case EPredicateType::CKBENTRY:
				// TODO - there is no effect with CKBEntry predicate in the condition for now...
				if (pred->mPrimary == "" || pred->mSecondary == "") {
					if (pred->mTruthLabel == ETruthLabel::INVALID) {
						salience += 3;
					}
					else {
						salience += 4;
					}
				}
				else if (pred->mTruthLabel == ETruthLabel::INVALID) {
					salience += 4;
				}
				else {
					salience += 5; // this means all entries were specified
				}
				break;
			case EPredicateType::SFDB_LABEL:
				// TODO - there is no effect with SFDB_LABEL predicate in the condition for now...
				if (pred->mPrimary == "" || pred->mSecondary == "") {
					if (pred->mSFDBLabel.type == ESFDBLabelType::INVALID) {
						salience += 3;
					}
					else {
						salience += 4;
					}
				}
				else if (pred->mSFDBLabel.type == ESFDBLabelType::INVALID) {
					salience += 4;
				}
				else {
					salience += 5; // this means all entries were specified
				}
				break;
		}
	}

	if (mLastSeenTime >= 0) {
		// this effect have been seen
		const auto cifManager = GetWorld()->GetGameInstance()->GetSubsystem<UCiFSubsystem>()->getInstance();
		const auto howLongBeforeItWasSeen = cifManager->mTime - mLastSeenTime;
		if (howLongBeforeItWasSeen < FEffectSaliencyValues::EFFECT_TOO_SOON) {
			salience -= FEffectSaliencyValues::EFFECT_TOO_SOON * 2 - 2 * howLongBeforeItWasSeen;
		}
	}

	mSalienceScore = salience;
	return salience;
}

bool UCiFEffect::evaluateCondition(UCiFGameObject* initiator, UCiFGameObject* responder, UCiFGameObject* other) const
{
	return mCondition->evaluate(initiator, responder, other);
}

void UCiFEffect::valuation(UCiFGameObject* initiator, UCiFGameObject* responder, UCiFGameObject* other) const
{
	mChange->valuation(initiator, responder, other);
}

bool UCiFEffect::hasCKBReference() const
{
	for (const auto p : mCondition->mPredicates) {
		if (p->mType == EPredicateType::CKBENTRY) {
			return true;
		}
	}
	return false;
}

bool UCiFEffect::hasSFDBLabel() const
{
	for (const auto p : mCondition->mPredicates) {
		if (p->mType == EPredicateType::SFDB_LABEL) {
			return true;
		}
	}
	return false;
}

bool UCiFEffect::isRoleRequired(const FName role) const
{
	return mCondition->isRoleRequired(role) || mChange->isRoleRequired(role);
}

UCiFPredicate* UCiFEffect::getCKBReferencePredicate() const
{
	for (const auto p : mCondition->mPredicates) {
		if (p->mType == EPredicateType::CKBENTRY) {
			return p;
		}
	}
	return nullptr;
}

void UCiFEffect::toString(FString& outStr) const
{
	outStr = mIsAccept ? "Accept\n" : "Reject\n";
	
	FString conditionStr;
	mCondition->toString(conditionStr);
	outStr += "Condition: " + conditionStr;
	outStr += "\n";

	FString changeStr;
	mChange->toString(changeStr);
	outStr += "Change: " + changeStr;
	outStr += "\n";
}

UCiFEffect* UCiFEffect::loadFromJson(const TSharedPtr<FJsonObject> json, const UObject* worldContextObject)
{
	auto e = NewObject<UCiFEffect>(const_cast<UObject*>(worldContextObject));
	
	e->mRejectId = json->GetNumberField(TEXT("_rejectID"));
	e->mIsAccept = json->GetBoolField(TEXT("_accept"));
	e->mInstantiationId = json->GetNumberField(TEXT("_instantiationID"));
	e->mReferenceAsNLG = FName(json->GetStringField(TEXT("PerformanceRealization")));


	e->mCondition = UCiFRule::loadFromJson(json->GetObjectField(TEXT("ConditionRule")), worldContextObject);
	e->mChange = UCiFRule::loadFromJson(json->GetObjectField(TEXT("ChangeRule")), worldContextObject);

	e->scoreSalience();

	return e;
}
