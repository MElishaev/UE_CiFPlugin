// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFPredicate.h"

#include "CiFCast.h"
#include "CiFManager.h"
#include "CiFRule.h"
#include "CiFSocialExchange.h"
#include "CiFSocialFactsDataBase.h"
#include "CiFSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "CiFGameObjectStatus.h"
#include "CiFCulturalKnowledgeBase.h"
#include "CiFRelationshipNetwork.h"
#include "CiFSocialNetwork.h"

UCiFPredicate::UCiFPredicate()
{
	clear();
}

bool UCiFPredicate::evaluate(const UCiFGameObject* c1, const UCiFGameObject* c2, const UCiFGameObject* c3, const UCiFSocialExchange* se)
{
	const UCiFManager* cifManager = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UCiFSubsystem>()->getInstance();

	/**
	 * Need to determine if the predicate's predicate variables reference
	 * roles (initiator,responder), generic variables (x,y,z), or 
	 * characters (edward, karen).
	 */

	// if Primary is not a reference to a game object, determine if it
	// is either a role or a generic variable
	const UCiFGameObject* first = cifManager->getGameObjectByName(mPrimary);
	if (!first) {
		const auto val = getValueOfPredicateVariable(mPrimary);
		if (val == "initiator") {
			first = c1;
		}
		else if (val == "responder") {
			first = c2;
		}
		else if (val == "other") {
			first = c3;
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("First variable doesn't bound to game object"));
		}
	}

	const UCiFGameObject* second = cifManager->getGameObjectByName(mSecondary);
	if (!second) {
		const auto val = getValueOfPredicateVariable(mSecondary);
		if (val == "initiator") {
			second = c1;
		}
		else if (val == "responder") {
			second = c2;
		}
		else if (val == "other") {
			second = c3;
		}
		else {
			second = nullptr;
			UE_LOG(LogTemp, Warning, TEXT("Second variable doesn't bound to game object"));
		}
	}

	bool isThird = true;
	const UCiFGameObject* third = cifManager->getGameObjectByName(mTertiary);
	if (!third) {
		const auto val = getValueOfPredicateVariable(mTertiary);
		if (val == "initiator") {
			third = c1;
		}
		else if (val == "responder") {
			third = c2;
		}
		else if (val == "other") {
			third = c3;
		}
		else {
			third = nullptr;
			isThird = false;
			UE_LOG(LogTemp, Warning, TEXT("Second variable doesn't bound to game object"));
		}
	}

	/*
	 * At this point only first has to be set. Any other bindings might
	 * not be valid depending on the type of the predicate. For example
	 * a CKBENTRY type could only have a first character variable
	 * specified and would work properly. If second or third are not set
	 * their value is set to null so the proper evaluation functions can
	 * determine how to hand the different cases of character variable
	 * specification.
	 */

	/**
	 * If isSFDB is true, we want to look over the Predicate's window 
	 * in the SFDB for this predicate's context. If found, the predicate
	 * is true. Otherwise, it is false.
	 * 
	 * This is the only evaluation needed for a Predicate with isSFDB
	 * being true. 
	 */
	if (mIsSFDB && mType != EPredicateType::SFDBLABEL) {
		return cifManager->mSFDB->isPredicateInHistory(this, c1, c2, c3);
	}

	/*
	 * If the predicate is intent, we want to check it against all of the 
	 * intent predicates in the intent rule in the passed-in social game.
	 * If this predicate matches any predicate in any rule of the intent
	 * rule vector of the social game, we return true.
	 * 
	 * Intents can only be networks and relationships.
	 */
	if (mIsIntent) {
		if (se->mIntents.Num() == 0) {
			UE_LOG(LogTemp, Warning, TEXT("intent predicate evaluation: the social game context has no intent"));
		}
		else {
			for (const auto rule : se->mIntents) {
				for (const auto pred : rule->mPredicates) {
					bool bMatch = false;
					if (mType == EPredicateType::STATUS) {
						bMatch = (pred->mStatusType == mStatusType) &&
							(pred->mPrimary == mPrimary) &&
							(pred->mSecondary == mSecondary) &&
							(pred->mIsNegated == mIsNegated);
					}
					else if (mType == EPredicateType::NETWORK) {
						bMatch = (pred->mNetworkType == mNetworkType) &&
							(pred->mComparatorType == mComparatorType) &&
							(pred->mPrimary == mPrimary) &&
							(pred->mSecondary == mSecondary) &&
							(pred->mIsNegated == mIsNegated);
					}
					else if (mType == EPredicateType::SFDBLABEL) {
						bMatch = (pred->mSFDBLabel == mSFDBLabel) &&
							(pred->mPrimary == mPrimary) &&
							(pred->mSecondary == mSecondary) &&
							(pred->mIsNegated == mIsNegated);
					}
					if (bMatch) {
						return true;
					}
				}
			}
		}
		/* We either have no predicate match to the social exchange's intent rules 
		 * or we are not a predicate type that can encompass intent. In
		 * either case, return false.
		 */
		return false;
	}

	if (mIsNumTimesUniquelyTruePred) {
		const bool bNumTimesResult = evalForNumberUniquelyTrue(first, second, third, se);
		return mIsNegated ? !bNumTimesResult : bNumTimesResult;
	}


	switch (mType) {
		case EPredicateType::TRAIT:
			return mIsNegated ? !evalTrait(first) : evalTrait(first);
		case EPredicateType::NETWORK:
			return evalNetwork(first, second);
		case EPredicateType::STATUS:
			return mIsNegated ? !evalStatus(first, second) : evalStatus(first, second);
		case EPredicateType::CKBENTRY:
			return evalCKBEntry(first, second);
		case EPredicateType::SFDBLABEL:
			return evalSFDBLabel(first, second, third);
		case EPredicateType::RELATIONSHIP:
			return mIsNegated ? !evalRelationship(first, second) : evalRelationship(first, second);
		default:
			UE_LOG(LogTemp, Warning, TEXT("evaluating a predicate without a recognized type of: %d"), mType);
	}

	return false;
}

bool UCiFPredicate::evalForNumberUniquelyTrue(const UCiFGameObject* c1,
                                              const UCiFGameObject* c2,
                                              const UCiFGameObject* c3,
                                              const UCiFSocialExchange* se)
{
	const UCiFManager* cifManager = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UCiFSubsystem>()->getInstance();

	int numTriesTrue = 0;
	bool predTrue = false;
	const UCiFGameObject* primaryCharacterOfConsideration;
	const UCiFGameObject* secondaryCharacterOfConsideration = nullptr;

	if (mNumTimesRoleSlot == ENumTimesRoleSlot::INVALID) {
		mNumTimesRoleSlot = ENumTimesRoleSlot::FIRST;
	}

	switch (mNumTimesRoleSlot) {
		case ENumTimesRoleSlot::FIRST:
			primaryCharacterOfConsideration = c1;
			break;
		case ENumTimesRoleSlot::SECOND:
			primaryCharacterOfConsideration = c2;
			break;
		case ENumTimesRoleSlot::BOTH:
			primaryCharacterOfConsideration = c1;
			secondaryCharacterOfConsideration = c2;
			break;
		default:
			mNumTimesRoleSlot = ENumTimesRoleSlot::FIRST;
			primaryCharacterOfConsideration = c1;
			UE_LOG(LogTemp, Warning, TEXT("Role slot is not recognized %d"), uint8(mNumTimesRoleSlot));
	}

	if (mNumTimesRoleSlot == ENumTimesRoleSlot::BOTH) {
		switch (mType) {
			case EPredicateType::CKBENTRY:
				TArray<FName> outArray;
				evalCKBEntryForObjects(primaryCharacterOfConsideration, secondaryCharacterOfConsideration, outArray);
				numTriesTrue = outArray.Num();
				break;
			case EPredicateType::SFDBLABEL:
				TArray<int> Out;
				cifManager->mSFDB->findLabelFromValues(Out,
				                                       mSFDBLabel.type,
				                                       primaryCharacterOfConsideration,
				                                       secondaryCharacterOfConsideration,
				                                       c3,
				                                       mWindowSize,
				                                       this);
				numTriesTrue = outArray.Num();
				break;
			default:
				UE_LOG(LogTemp, Warning, TEXT("Doesn't make sense consider 'both' role type for pred types not CKB or SFDB %d"), mType);
		}
	}
	else {
		for (const auto c : cifManager->mCast->mCharacters) {
			predTrue = false;
			if (c->mObjectName != primaryCharacterOfConsideration->mObjectName) {
				switch (mType) {
					case EPredicateType::TRAIT:
						predTrue = evalTrait(primaryCharacterOfConsideration);
						break;
					case EPredicateType::NETWORK:
						if (mNumTimesRoleSlot == ENumTimesRoleSlot::SECOND) {
							predTrue = evalNetwork(c, primaryCharacterOfConsideration);
						}
						else {
							predTrue = evalNetwork(primaryCharacterOfConsideration, c);
						}
						break;
					case EPredicateType::STATUS:
						if (mNumTimesRoleSlot == ENumTimesRoleSlot::SECOND) {
							predTrue = evalStatus(c, primaryCharacterOfConsideration);
						}
						else {
							predTrue = evalStatus(primaryCharacterOfConsideration, c);
						}
						break;
					case EPredicateType::CKBENTRY:
						predTrue = evalCKBEntry(primaryCharacterOfConsideration, c);
						break;
					case EPredicateType::SFDBLABEL:
						if (mNumTimesRoleSlot == ENumTimesRoleSlot::SECOND) {
							TArray<int> out;
							cifManager->mSFDB->findLabelFromValues(out,
							                                       mSFDBLabel.type,
							                                       c,
							                                       primaryCharacterOfConsideration,
							                                       nullptr,
							                                       mWindowSize,
							                                       this);
							numTriesTrue += out.Num();
						}
						else {
							TArray<int> out;
							cifManager->mSFDB->findLabelFromValues(out,
							                                       mSFDBLabel.type,
							                                       primaryCharacterOfConsideration,
							                                       c,
							                                       nullptr,
							                                       mWindowSize,
							                                       this);
							numTriesTrue += out.Num();
						}
						break;
					case EPredicateType::RELATIONSHIP:
						predTrue = evalRelationship(primaryCharacterOfConsideration, c);
						break;
					default:
						UE_LOG(LogTemp, Warning, TEXT("evaluating a predicate without a recoginzed type of: %d"), mType);
				}
				if (predTrue) numTriesTrue++;
			}
		}
	}

	// This is a special case for where we want to count numTimesTrue for contexts labels that don't have the nonPrimary role specified 
	if (mType == EPredicateType::SFDBLABEL && mIsNumTimesUniquelyTruePred) {
		if (mNumTimesRoleSlot == ENumTimesRoleSlot::FIRST) {
			TArray<int> out;
			cifManager->mSFDB->findLabelFromValues(out,
			                                       mSFDBLabel.type,
			                                       primaryCharacterOfConsideration,
			                                       nullptr,
			                                       nullptr,
			                                       mWindowSize,
			                                       this);
			numTriesTrue += out.Num();
		}
	}

	return numTriesTrue >= mNumTimesUniquelyTrue;
}

void UCiFPredicate::evalCKBEntryForObjects(const UCiFGameObject* first, const UCiFGameObject* second, TArray<FName>& outArray) const
{
	const UCiFManager* cifManager = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UCiFSubsystem>()->getInstance();
	auto ckb = cifManager->mCKB;

	if (!second) {
		//determine if the single character constraints results in a match
		ckb->findItems(first->mObjectName, outArray, mFirstSubjectiveLink, mTruthLabel);
	}
	else {
		TArray<FName> firstResults;
		TArray<FName> secondResults;

		//determine if the two character constraints result in a match
		//1. find first matches
		ckb->findItems(first->mObjectName, firstResults, mTruthLabel, mFirstSubjectiveLink);

		if (mSecondSubjectiveLink == "") {
			outArray = firstResults;
			return;
		}

		//2. find second matches
		ckb->findItems(second->mObjectName, secondResults, mTruthLabel, mSecondSubjectiveLink);
		//3. see if any of first's matches intersect second's matches.
		for (int32 i = 0; i < firstResults.Num(); ++i) {
			for (int32 j = 0; j < secondResults.Num(); ++j) {
				if (firstResults[i] == secondResults[j]) {
					outArray.Add(firstResults[i]);
				}
			}
		}
	}
}

bool UCiFPredicate::evalTrait(const UCiFGameObject* first) const
{
	return first->hasTrait(mTrait);
}

bool UCiFPredicate::evalNetwork(const UCiFGameObject* first, const UCiFGameObject* second) const
{
	const UCiFManager* cifManager = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UCiFSubsystem>()->getInstance();

	const uint8 firstNetworkID = first->mNetworkId;
	uint8 secondNetworkID = 0;
	if (second) {
		secondNetworkID = second->mNetworkId;
	}

	// get the proper network
	UCiFSocialNetwork* network = cifManager->getSocialNetworkByType(mNetworkType);
	if (!network) {
		UE_LOG(LogTemp, Error, TEXT("Invalid network type %d"), mNetworkType);
		return false;
	}

	switch (mComparatorType) {
		case EComparatorType::LESS_THAN:
			if (network->getWeight(firstNetworkID, secondNetworkID) < mNetworkValue) {
				return true && !mIsNegated;
			}
			break;
		case EComparatorType::GREATER_THAN:
			if (network->getWeight(firstNetworkID, secondNetworkID) >= mNetworkValue) {
				return true && !mIsNegated;
			}
			break;
		case EComparatorType::AVERAGE_OPINION:
			if (network->getAverageOpinion(firstNetworkID) > mNetworkValue) {
				return true && !mIsNegated;
			}
			break;
		case EComparatorType::FRIENDS_OPINION:
		case EComparatorType::DATING_OPINION:
		case EComparatorType::ENEMIES_OPINION:
			{
				// first's friends/enemies/date's opinions on second
				// so if im John and my friend is Dani, I'm checking Dani's opinion on Karen
				const auto relType = comparatorTypeToRelationshipType(mComparatorType);
				uint16 amountCounted = 0;
				float sum = 0;
				const auto rel = cifManager->mRelationshipNetworks;
				for (const auto c : cifManager->mCast->mCharacters) {
					if ((c->mObjectName != first->mObjectName) && (c->mObjectName != second->mObjectName)) {
						if (rel->getRelationship(relType, c, static_cast<const UCiFCharacter*>(first))) {
							amountCounted++;
							sum += network->getWeight(c->mNetworkId, second->mNetworkId);
						}
					}
				}

				if (amountCounted < 1) {
					return false;
				}

				if ((sum / amountCounted) < mNetworkValue) {
					return true && !mIsNegated;
				}
				break;
			}
		default:
			UE_LOG(LogTemp, Error, TEXT("Invalid comparotr type %d"), mComparatorType);
			return false;
	}

	return false || mIsNegated;
}

bool UCiFPredicate::evalStatus(const UCiFGameObject* first, const UCiFGameObject* second) const
{
	return first->hasStatus(mStatusType, second);
}

bool UCiFPredicate::evalCKBEntry(const UCiFGameObject* first, const UCiFGameObject* second) const
{
	const UCiFManager* cifManager = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UCiFSubsystem>()->getInstance();

	if (!second) {
		// determine if the single character constraints result in a match
		TArray<FName> outputItems;
		cifManager->mCKB->findItems(first->mObjectName, outputItems, mFirstSubjectiveLink, mTruthLabel);
		return outputItems.Num() > 0;
	}

	// determine if the two character constraints result in a match
	TArray<FName> firstItems;
	cifManager->mCKB->findItems(first->mObjectName, firstItems, mFirstSubjectiveLink, mTruthLabel);
	TArray<FName> secondItems;
	cifManager->mCKB->findItems(second->mObjectName, secondItems, mFirstSubjectiveLink, mTruthLabel);

	// see if first's matches intersect with second's
	for (const auto item1 : firstItems) {
		for (const auto item2 : secondItems) {
			if (item1 == item2) {
				return true;
			}
		}
	}
	return false;
}

bool UCiFPredicate::evalRelationship(const UCiFGameObject* first, const UCiFGameObject* second) const
{
	const UCiFManager* cifManager = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UCiFSubsystem>()->getInstance();
	const auto rel = cifManager->mRelationshipNetworks;
	return rel->getRelationship(mRelationshipType, static_cast<const UCiFCharacter*>(first), static_cast<const UCiFCharacter*>(second));
}

bool UCiFPredicate::evalSFDBLabel(const UCiFGameObject* first, const UCiFGameObject* second, const UCiFGameObject* third) const
{
	const UCiFManager* cifManager = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UCiFSubsystem>()->getInstance();

	if (isSFDBLabelCategory()) {
		for (const auto sfdbLabel : UCiFSocialFactsDataBase::mSFDBLabelCategories[mSFDBLabel.type]) {
			TArray<int> outIndices;
			cifManager->mSFDB->findLabelFromValues(outIndices, sfdbLabel, first, second, nullptr, mWindowSize, this);
			if (!outIndices.IsEmpty()) {
				return !mIsNegated;
			}
		}
	}
	else {
		TArray<int> outIndices;
		cifManager->mSFDB->findLabelFromValues(outIndices, mSFDBLabel.type, first, second, nullptr, mWindowSize, this);
		if (!outIndices.IsEmpty()) {
			return !mIsNegated;
		}
	}
	return !mIsNegated;
}

EIntentType UCiFPredicate::getIntentType()
{
	if (!mIsIntent) {
		return EIntentType::INVALID;
	}

	if (mType == EPredicateType::NETWORK) {
		switch (mNetworkType) {
			case ESocialNetworkType::SN_BUDDY:
				{
					if (mOperatorType == EOperatorType::INCREASE) {
						return EIntentType::BUDDY_UP;
					}
					return EIntentType::BUDDY_DOWN;
				}
			case ESocialNetworkType::SN_ROMANCE:
				{
					if (mOperatorType == EOperatorType::INCREASE) {
						return EIntentType::ROMANCE_UP;
					}
					return EIntentType::ROMANCE_DOWN;
				}
		}
	}
	else if (mType == EPredicateType::RELATIONSHIP) {
		switch (mRelationshipType) {
			case ERelationshipType::FRIENDS: return mIsNegated ? EIntentType::FRIENDS : EIntentType::END_FRIENDS;
			case ERelationshipType::DATING: return mIsNegated ? EIntentType::DATING : EIntentType::END_DATING;
			case ERelationshipType::ENEMIES: return mIsNegated ? EIntentType::ENEMIES : EIntentType::END_ENEMIES;
		}
	}

	return EIntentType::INVALID;
}

FName UCiFPredicate::getValueOfPredicateVariable(const FName var) const
{
	if (var == "i" || var == "initiator") return "initiator";
	if (var == "r" || var == "responder") return "responder";
	if (var == "o" || var == "other") return "other";

	UE_LOG(LogTemp, Warning, TEXT("Couldn't find value for predicate variable %s"), *(var.ToString()));
	return "";
}

ERelationshipType UCiFPredicate::comparatorTypeToRelationshipType(const EComparatorType comparatorType) const
{
	switch (comparatorType) {
		case EComparatorType::FRIENDS_OPINION: return ERelationshipType::FRIENDS;
		case EComparatorType::DATING_OPINION: return ERelationshipType::DATING;
		case EComparatorType::ENEMIES_OPINION: return ERelationshipType::ENEMIES;
		case
		default:
			UE_LOG(LogTemp, Warning, TEXT("Compartor type %d is not of a relationship networks"), comparatorType);
			return ERelationshipType::FRIENDS; // return this value as a default
	}
}

bool UCiFPredicate::isSFDBLabelCategory() const
{
	return (mType == EPredicateType::SFDBLABEL) && (mSFDBLabel.type <= ESFDBLabelType::CAT_LAST);
}

void UCiFPredicate::clear()
{
	mPrimary = "";
	mSecondary = "";
	mTertiary = "";
	mIsSFDB = false;
	mIsIntent = false;
	mIsNegated = false;
	mType = EPredicateType::INVALID;
	mStatusType = EStatus::INVALID;
	mWindowSize = 0;
	mSFDBOrder = 0;
	mIsNumTimesUniquelyTruePred = false; // Flag that specifies if this is a "number of times this pred is uniquely true" type pred
	mNumTimesRoleSlot = ENumTimesRoleSlot::INVALID;
}
