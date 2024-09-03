// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFEffect.h"

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

	for (const auto pred : mChange->mPredicates) {
		if (pred->mType == EPredicateType::SFDB_LABEL) {
			salience += 6;
		}
	}

	for (const auto pred : mCondition->mPredicates) {
		switch (pred->mType) {
			case EPredicateType::INVALID:
				break;
			case EPredicateType::TRAIT:
				if (pred->mIsNegated) {
					salience += 1;
				}
				else {
					salience += 4;
				}
				break;
			case EPredicateType::NETWORK:
				switch (pred->mComparatorType) {
					case EComparatorType::LESS_THAN:
						if (pred->mNetworkValue == 34) {
							// TODO - change the hardcoded value to some
							salience += FEffectSaliencyValues::LOW_NETWORK_SALIENCE;
						}
						else if (pred->mNetworkValue == 66) {
							salience += FEffectSaliencyValues::HIGH_NETWORK_SALIENCE;
						}
						break;
					case EComparatorType::GREATER_THAN:
						if (pred->mNetworkValue == 33) {
							// TODO - change the hardcoded value to some
							salience += FEffectSaliencyValues::MEDIUM_NETWORK_SALIENCE;
						}
						else if (pred->mNetworkValue == 66) {
							salience += FEffectSaliencyValues::HIGH_NETWORK_SALIENCE;
						}
						break;
					default:
						salience += FEffectSaliencyValues::UNRECOGNIZED_NETWORK_SALIENCE;
				}
				break;
			case EPredicateType::RELATIONSHIP:
				salience += pred->mIsNegated ? 1 : 3;
				break;
			case EPredicateType::STATUS:
				if (pred->mIsNegated) {
					salience += 1;
				}
				else {
					salience += 3;
				}
				break;
			case EPredicateType::CKBENTRY:
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
				if (pred->mPrimary == "" || pred->mSecondary == "") {
					if (static_cast<int>(pred->mSFDBLabel.type) < 0) {
						salience += 3;
					}
					else {
						salience += 4;
					}
				}
				else if (static_cast<int>(pred->mSFDBLabel.type) < 0) {
					salience += 4;
				}
				else {
					salience += 5; // this means all entries were specified
				}
				break;
		}

		if (pred->mRelationshipType == ERelationshipType::ENEMIES || pred->mRelationshipType == ERelationshipType::DATING) {
			salience += 3;
		}
	}

	if (mLastSeenTime >= 0) {
		// this effect have been seen
		const auto cifManager = GetWorld()->GetGameInstance()->GetSubsystem<UCiFSubsystem>()->getInstance();
		const auto howLongBeforeItWasSeen = cifManager->mTime - mLastSeenTime;
		if (howLongBeforeItWasSeen < FEffectSaliencyValues::EFFECT_TOO_SOON) {
			salience -= FEffectSaliencyValues::EFFECT_TOO_SOON * 2.5 - 2 * howLongBeforeItWasSeen;
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
	outStr = mIsAccept ? "Accept: " : "Reject: ";
	
	FString conditionStr;
	mCondition->toString(conditionStr);
	outStr += conditionStr;
	outStr += " | ";

	FString changeStr;
	mChange->toString(changeStr);
	outStr += changeStr;
}

UCiFEffect* UCiFEffect::loadFromJson(const TSharedPtr<FJsonObject> json, const UObject* worldContextObject)
{
	auto e = NewObject<UCiFEffect>(const_cast<UObject*>(worldContextObject));
	
	e->mRejectId = json->GetNumberField("_rejectID");
	e->mIsAccept = json->GetBoolField("_accept");
	e->mInstantiationId = json->GetNumberField("_instantiationID");
	e->mReferenceAsNLG = FName(json->GetStringField("PerformanceRealization"));


	e->mCondition = UCiFRule::loadFromJson(json->GetObjectField("ConditionRule"), worldContextObject);
	e->mChange = UCiFRule::loadFromJson(json->GetObjectField("ChangeRule"), worldContextObject);

	e->scoreSalience();

	return e;
}
