﻿// Fill out your copyright notice in the Description page of Project Settings.


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
	const UCiFManager* cifManager = GetWorld()->GetGameInstance()->GetSubsystem<UCiFSubsystem>()->getInstance();

	/**
	 * Need to determine if the predicate's predicate variables reference
	 * roles (initiator,responder), generic variables (x,y,z), or 
	 * characters (edward, karen).
	 */

	// if Primary is not a reference to a game object, determine if it
	// is either a role or a generic variable
	UCiFGameObject* first = cifManager->getGameObjectByName(mPrimary);
	UCiFGameObject* second = cifManager->getGameObjectByName(mSecondary);
	UCiFGameObject* third = cifManager->getGameObjectByName(mTertiary);
	determinePredicatesVars(first, second, third, const_cast<UCiFGameObject*>(c1), const_cast<UCiFGameObject*>(c2), const_cast<UCiFGameObject*>(c3));

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
	if (mIsSFDB && mType != EPredicateType::SFDB_LABEL) {
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
					else if (mType == EPredicateType::SFDB_LABEL) {
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
		case EPredicateType::SFDB_LABEL:
			return evalSFDBLabel(first, second, third);
		case EPredicateType::RELATIONSHIP:
			return mIsNegated ? !evalRelationship(first, second) : evalRelationship(first, second);
		default:
			UE_LOG(LogTemp, Warning, TEXT("evaluating a predicate without a recognized type of: %d"), mType);
	}

	return false;
}

void UCiFPredicate::valuation(UCiFGameObject* x, UCiFGameObject* y, UCiFGameObject* z)
{
	/**
	 * Need to determine if the predicate's predicate variables reference
	 * roles (initiator,responder), generic variables (x,y,z), or 
	 * characters (edward, karen).
	 */

	//if primary is not a reference to a character, determine if 
	//it is either a role or a generic variable
	const auto cifManager = GetWorld()->GetGameInstance()->GetSubsystem<UCiFSubsystem>()->getInstance();
	auto first = cifManager->getGameObjectByName(mPrimary);
	auto second = cifManager->getGameObjectByName(mSecondary);
	auto third = cifManager->getGameObjectByName(mTertiary);
	determinePredicatesVars(first, second, third, x, y, z);

	/*
	 * At this point only first has to be set. Any other bindings might
	 * not be valid depending on the type of the predicate. For example
	 * a CKBENTRY type could only have a first character variable
	 * specified and would work properly. If second or third are not set
	 * their value is set to null so the proper evaluation functions can
	 * determine how to handle the different cases of character variable
	 * specification.
	 */
	switch (mType) {
		case EPredicateType::TRAIT:
			UE_LOG(LogTemp, Warning, TEXT("Traits cannot be subject to valuation"));
			break;
		case EPredicateType::NETWORK:
			updateNetwork(first, second);
			break;
		case EPredicateType::RELATIONSHIP:
			updateRelationship(first, second);
			break;
		case EPredicateType::STATUS:
			updateStatus(first, second);
			break;
		case EPredicateType::CKBENTRY:
			UE_LOG(LogTemp, Warning, TEXT("CKBENTRIES cannot be subject to valuation"));
			break;
		case EPredicateType::SFDB_LABEL:
			UE_LOG(LogTemp, Warning, TEXT("SFDBLABELs cannot be subject to valuation"));
			break;
		default:
			UE_LOG(LogTemp, Warning, TEXT("preforming valuation a predicate without a recoginzed type %d"), mType);
	}
}

void UCiFPredicate::determinePredicatesVars(UCiFGameObject*& first,
                                            UCiFGameObject*& second,
                                            UCiFGameObject*& third,
                                            UCiFGameObject* x,
                                            UCiFGameObject* y,
                                            UCiFGameObject* z) const
{
	if (!first) {
		const auto val = getRoleValue(mPrimary);
		if (val == "initiator" || val == "x") {
			first = x;
		}
		else if (val == "responder" || val == "y") {
			first = y;
		}
		else if (val == "other" || val == "z") {
			first = z;
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("first variable was not bound to a character"));
		}
	}

	if (!second) {
		const auto val = getRoleValue(mSecondary);
		if (val == "initiator" || val == "x") {
			second = x;
		}
		else if (val == "responder" || val == "y") {
			second = y;
		}
		else if (val == "other" || val == "z") {
			second = z;
		}
	}

	if (!third) {
		const auto val = getRoleValue(mTertiary);
		if (val == "initiator" || val == "x") {
			third = x;
		}
		else if (val == "responder" || val == "y") {
			third = y;
		}
		else if (val == "other" || val == "z") {
			third = z;
		}
	}
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
				{
					TArray<FName> outArray;
					evalCKBEntryForObjects(primaryCharacterOfConsideration, secondaryCharacterOfConsideration, outArray);
					numTriesTrue = outArray.Num();
					break;
				}
			case EPredicateType::SFDB_LABEL:
				{
					TArray<int> out{};
					cifManager->mSFDB->findLabelFromValues(out,
					                                       mSFDBLabel.type,
					                                       primaryCharacterOfConsideration,
					                                       secondaryCharacterOfConsideration,
					                                       c3,
					                                       mWindowSize,
					                                       this);
					numTriesTrue = out.Num();
				}
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
					case EPredicateType::SFDB_LABEL:
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
	if (mType == EPredicateType::SFDB_LABEL && mIsNumTimesUniquelyTruePred) {
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
		ckb->findItems(first->mObjectName, firstResults, mFirstSubjectiveLink, mTruthLabel);

		if (mSecondSubjectiveLink == ESubjectiveLabel::INVALID) {
			outArray = firstResults;
			return;
		}

		//2. find second matches
		ckb->findItems(second->mObjectName, secondResults, mSecondSubjectiveLink, mTruthLabel);
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
	if ((first && first->mGameObjectType != ECiFGameObjectType::CHARACTER) ||
		(second && second->mGameObjectType != ECiFGameObjectType::CHARACTER)) {
		return false;
	}
	const UCiFManager* cifManager = GetWorld()->GetGameInstance()->GetSubsystem<UCiFSubsystem>()->getInstance();

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
	if (first->mGameObjectType != ECiFGameObjectType::CHARACTER || second->mGameObjectType != ECiFGameObjectType::CHARACTER) {
		return false;
	}
	const UCiFManager* cifManager = GetWorld()->GetGameInstance()->GetSubsystem<UCiFSubsystem>()->getInstance();
	const auto rel = cifManager->mRelationshipNetworks;
	return rel->getRelationship(mRelationshipType, static_cast<const UCiFCharacter*>(first), static_cast<const UCiFCharacter*>(second));
}

bool UCiFPredicate::evalSFDBLabel(const UCiFGameObject* first, const UCiFGameObject* second, const UCiFGameObject* third) const
{
	const UCiFManager* cifManager = GetWorld()->GetGameInstance()->GetSubsystem<UCiFSubsystem>()->getInstance();

	if (isSFDBLabelCategory()) {
		for (const auto sfdbLabel : UCiFSocialFactsDataBase::mSFDBLabelCategories[mSFDBLabel.type].mCategoryLabels) {
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
	return mIsNegated;
}

bool UCiFPredicate::equalsValuationStructure(const UCiFPredicate* p1, const UCiFPredicate* p2)
{
	if ((p1->mType != p2->mType) || (p1->mIsIntent != p2->mIsIntent) ||
		(p1->mIsNegated != p2->mIsNegated) || (p1->mIsNumTimesUniquelyTruePred != p2->mIsNumTimesUniquelyTruePred) ||
		(p1->mNumTimesUniquelyTrue != p2->mNumTimesUniquelyTrue) || (p1->mNumTimesRoleSlot != p2->mNumTimesRoleSlot)) {
		return false;
	}

	switch (p1->mType) {
		case EPredicateType::INVALID: return false;
		case EPredicateType::TRAIT:
			if (p1->mTrait != p2->mTrait) return false;
			break;
		case EPredicateType::NETWORK:
			if ((p1->mNetworkType != p2->mNetworkType) ||
				(p1->mComparatorType != p2->mComparatorType))
				return false;
			break;
		case EPredicateType::RELATIONSHIP:
			if (p1->mRelationshipType != p2->mRelationshipType) return false;
			break;
		case EPredicateType::STATUS:
			if (p1->mStatusType != p2->mStatusType) return false;
			break;
		case EPredicateType::CKBENTRY:
			if ((p1->mFirstSubjectiveLink != p2->mFirstSubjectiveLink) ||
				(p1->mSecondSubjectiveLink != p2->mSecondSubjectiveLink) ||
				(p1->mTruthLabel != p2->mTruthLabel))
				return false;
			break;
		case EPredicateType::SFDB_LABEL:
			if ((p1->mSFDBLabel != p2->mSFDBLabel) ||
				(p1->mIsNegated != p2->mIsNegated))
				return false;
			break;
		default:
			UE_LOG(LogTemp, Warning, TEXT("Unknown predicate type %d"), p1->mType);
			return false;
	}
	return true;
}

EIntentType UCiFPredicate::getIntentType()
{
	if (!mIsIntent) {
		return EIntentType::INVALID;
	}

	if (mType == EPredicateType::NETWORK) {
		switch (mNetworkType) {
			case ESocialNetworkType::BUDDY:
				{
					if (mComparatorType == EComparatorType::INCREASE) {
						return EIntentType::BUDDY_UP;
					}
					return EIntentType::BUDDY_DOWN;
				}
			case ESocialNetworkType::ROMANCE:
				{
					if (mComparatorType == EComparatorType::INCREASE) {
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

FName UCiFPredicate::getRoleValue(const FName val) const
{
	const auto cifManager = GetWorld()->GetGameInstance()->GetSubsystem<UCiFSubsystem>()->getInstance();

	if (val == "init" || val == "initiator" || val == "i") {
		return "initiator";
	}
	if (val == "res" || val == "responder" || val == "r") {
		return "responder";
	}
	if (val == "o" || val == "oth" || val == "other") {
		return "other";
	}
	if (val == "x" || val == "y" || val == "z") {
		return val;
	}
	if (val == "") {
		return "";
	}
	if (cifManager->getGameObjectByName(val)) {
		return val;
	}
	UE_LOG(LogTemp, Warning, TEXT("Primary value of predicated is %s"), *val.ToString());

	return "";
}

FName UCiFPredicate::getSecondaryCharacterNameFromVariables(const UCiFGameObject* initiator,
                                                            const UCiFGameObject* responder,
                                                            const UCiFGameObject* other) const
{
	const auto roleVal = getRoleValue(mSecondary);

	if (roleVal == "initiator") return initiator->mObjectName;
	if (roleVal == "responder") return responder->mObjectName;
	if (roleVal == "other") return other->mObjectName;
	if (roleVal == "") return "";

	return mSecondary;
}

FName UCiFPredicate::getPrimaryCharacterNameFromVariables(const UCiFGameObject* initiator,
                                                          const UCiFGameObject* responder,
                                                          const UCiFGameObject* other) const
{
	const auto roleVal = getRoleValue(mPrimary);

	if (roleVal == "initiator") return initiator->mObjectName;
	if (roleVal == "responder") return responder->mObjectName;
	if (roleVal == "other") return other->mObjectName;
	if (roleVal == "") return "";

	return mPrimary;
}

void UCiFPredicate::setTraitPredicate(const FName first, const ETrait trait, const bool isNegated, const bool isSFDB)
{
	mType = EPredicateType::TRAIT;
	mTrait = trait;
	mPrimary = first;
	mIsNegated = isNegated;
	mIsSFDB = isSFDB;
}

void UCiFPredicate::setNetworkPredicate(const FName first,
                                        const FName second,
                                        const EComparatorType comp,
                                        const int8 networkValue,
                                        const ESocialNetworkType networkType,
                                        const bool isNegated,
                                        const bool isSFDB)
{
	mType = EPredicateType::NETWORK;
	mNetworkValue = networkValue;
	mPrimary = first;
	mSecondary = second;
	mComparatorType = comp;
	mNetworkType = networkType;
	mIsNegated = isNegated;
	mIsSFDB = isSFDB;
}

void UCiFPredicate::setStatusPredicate(const FName first,
                                       const FName second,
                                       const EStatus status,
                                       const int32 duration,
                                       const bool isSFDB,
                                       const bool isNegated)
{
	mType = EPredicateType::STATUS;
	mPrimary = first;
	mSecondary = second;
	mStatusType = status;
	mStatusDuration = duration;
	mIsNegated = isNegated;
	mIsSFDB = isSFDB;
}

void UCiFPredicate::setCKBPredicate(const FName first,
                                    const FName second,
                                    const ESubjectiveLabel firstSub,
                                    const ESubjectiveLabel secondSub,
                                    const ETruthLabel truth,
                                    const bool isNegated)
{
	mType = EPredicateType::CKBENTRY;
	mPrimary = first;
	mSecondary = second;
	mFirstSubjectiveLink = firstSub;
	mSecondSubjectiveLink = secondSub;
	mTruthLabel = truth;
	mIsNegated = isNegated;
}

void UCiFPredicate::setSFDBLabelPredicate(const FName first,
                                          const FName second,
                                          const ESFDBLabelType labelType,
                                          const uint32 window,
                                          const bool isNegated)
{
	mType = EPredicateType::SFDB_LABEL;
	mPrimary = first;
	mSecondary = second;
	mIsSFDB = true;
	mIsNegated = isNegated;
	mSFDBLabel.type = labelType;
	mSFDBLabel.from = first;
	mSFDBLabel.to = second;
	mWindowSize = window;
}

void UCiFPredicate::setRelationshipPredicate(const FName first,
                                             const FName second,
                                             const ERelationshipType relType,
                                             const bool isNegated,
                                             const bool isSFDB)
{
	mType = EPredicateType::RELATIONSHIP;
	mPrimary = first;
	mSecondary = second;
	mRelationshipType = relType;
	mIsNegated = isNegated;
	mIsSFDB = isSFDB;
}

void UCiFPredicate::updateNetwork(UCiFGameObject* first, UCiFGameObject* second)
{
	checkf(first->mGameObjectType == ECiFGameObjectType::CHARACTER && second->mGameObjectType == ECiFGameObjectType::CHARACTER,
	       TEXT("first and second must be characters in a social network valuation"));
	const UCiFManager* cifManager = GetWorld()->GetGameInstance()->GetSubsystem<UCiFSubsystem>()->getInstance();

	const auto net = cifManager->getSocialNetworkByType(mNetworkType);

	auto netEnum = StaticEnum<ESocialNetworkType>();
	
	switch (mComparatorType) {
		case EComparatorType::LESS_THAN:
		case EComparatorType::GREATER_THAN:
		case EComparatorType::AVERAGE_OPINION:
		case EComparatorType::FRIENDS_OPINION:
		case EComparatorType::DATING_OPINION:
		case EComparatorType::ENEMIES_OPINION:
			UE_LOG(LogTemp, Warning, TEXT("Not relevant in this method"));
			break;
		case EComparatorType::INCREASE:
			net->addWeight(first->mNetworkId, second->mNetworkId, mNetworkValue);
			UE_LOG(LogTemp, Warning, TEXT("Network %s: %s-->%s +%d"), *(netEnum->GetNameStringByValue(int(net->mType))),
				*(first->mObjectName.ToString()), *(second->mObjectName.ToString()), mNetworkValue);
			break;
		case EComparatorType::DECREASE:
			net->addWeight(first->mNetworkId, second->mNetworkId, -mNetworkValue);
			UE_LOG(LogTemp, Warning, TEXT("Network %s: %s-->%s -%d"), *(netEnum->GetNameStringByValue(int(net->mType))),
				*(first->mObjectName.ToString()), *(second->mObjectName.ToString()), mNetworkValue);
			break;
	}
}

void UCiFPredicate::updateStatus(UCiFGameObject* first, UCiFGameObject* second) const
{
	const auto statusEnum = StaticEnum<EStatus>();
	checkf(statusEnum->GetValueAsName(mStatusType) != "None", TEXT("Not found the specified status"));
	if (mIsNegated) {
		if (second) {
			UE_LOG(LogTemp,
			       Log,
			       TEXT("%s removing status %s from %s"),
			       *(first->mObjectName.ToString()),
			       *(statusEnum->GetValueAsString(mStatusType)),
			       *(second->mObjectName.ToString()));
			first->removeStatus(mStatusType, second->mObjectName);
		}
		else {
			UE_LOG(LogTemp,
			       Log,
			       TEXT("%s removing status %s"),
			       *(first->mObjectName.ToString()),
			       *(statusEnum->GetValueAsString(mStatusType)));
			first->removeStatus(mStatusType);
		}
	}
	else {
		if (second) {
			UE_LOG(LogTemp,
			       Log,
			       TEXT("Added status: %s %s %s"),
			       *(first->mObjectName.ToString()),
			       *(statusEnum->GetValueAsString(mStatusType)),
			       *(second->mObjectName.ToString()));
			first->addStatus(mStatusType, mStatusDuration, second->mObjectName);
		}
		else {
			UE_LOG(LogTemp,
			       Log,
			       TEXT("Added status: %s is %s for duration %d"),
			       *(first->mObjectName.ToString()),
			       *(statusEnum->GetValueAsString(mStatusType)),
			       mStatusDuration);
			first->addStatus(mStatusType, mStatusDuration);
		}
	}
}

void UCiFPredicate::updateRelationship(UCiFGameObject* first, UCiFGameObject* second) const
{
	checkf(first != nullptr && second != nullptr &&
	       first->mGameObjectType == ECiFGameObjectType::CHARACTER && second->mGameObjectType == ECiFGameObjectType::CHARACTER,
	       TEXT("Something went wrong. updating relationships while one of the parties is null, or objects are not characters"));

	const UCiFManager* cifManager = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UCiFSubsystem>()->getInstance();

	auto firstAsChar = static_cast<UCiFCharacter*>(first);
	auto secondAsChar = static_cast<UCiFCharacter*>(second);

	auto relEnum = StaticEnum<ERelationshipType>();
	auto relStr = relEnum->GetValueAsString(mRelationshipType);

	auto net = cifManager->mRelationshipNetworks;
	if (mIsNegated) {
		net->removeRelationship(mRelationshipType, firstAsChar, secondAsChar);
		UE_LOG(LogTemp,
		       Log,
		       TEXT("%s relationship removed between %s and %s"),
		       *relStr,
		       *(first->mObjectName.ToString()),
		       *(second->mObjectName.ToString()))
	}
	else {
		net->setRelationship(mRelationshipType, firstAsChar, secondAsChar);
		UE_LOG(LogTemp,
		       Log,
		       TEXT("%s relationship added between %s and %s"),
		       *relStr,
		       *(first->mObjectName.ToString()),
		       *(second->mObjectName.ToString()))
	}
}

void UCiFPredicate::toIntentNLGString(FString& outputStr)
{
	outputStr = "";
	if (mIsIntent) {
		outputStr = mName.ToString();
		// const auto predTypeEnum = StaticEnum<EPredicateType>();
		// auto typeString = predTypeEnum->GetValueAsString(mType);
		// const auto intentTypeEnum = StaticEnum<EIntentType>();
		// auto intentTypeString = intentTypeEnum->GetValueAsString(mIntentType);
		//
		// switch (mIntentType) {
		// 	case EIntentType::COOL_UP:
		// 	case EIntentType::BUDDY_UP:
		// 	case EIntentType::ROMANCE_UP:
		// 		outputStr = "(Improve " + typeString + " network)";
		// 		break;
		// 	case EIntentType::COOL_DOWN:
		// 	case EIntentType::BUDDY_DOWN:
		// 	case EIntentType::ROMANCE_DOWN:
		// 		outputStr = "(Weaken " + typeString + " network)";
		// 		break;
		// 	case EIntentType::DATING:
		// 	case EIntentType::ENEMIES:
		// 	case EIntentType::FRIENDS:
		// 		outputStr = "(Add status " + intentTypeString + ")";
		// 		break;
		// 	case EIntentType::END_DATING:
		// 	case EIntentType::END_ENEMIES:
		// 	case EIntentType::END_FRIENDS:
		// 		outputStr = "(Remove status " + intentTypeString + ")";
		// 		break;
		// 	default:
		// 		UE_LOG(LogTemp, Warning, TEXT("Unrecognized intent type: %d"), mIntentType);
		// }
	}
}

FString UCiFPredicate::toNLG(const FName initiatorName, const FName responderName, const FName otherName)
{
	FString outStr;
	if (!mIsNumTimesUniquelyTruePred) {
		switch (mType) {
			case EPredicateType::TRAIT:
				outStr = traitPredToNLG(initiatorName, responderName, otherName);
				break;
			case EPredicateType::NETWORK:
				outStr = networkPredToNLG(initiatorName, responderName, otherName);
				break;
			case EPredicateType::RELATIONSHIP:
				outStr = relationshipPredToNLG(initiatorName, responderName, otherName);
				break;
			case EPredicateType::STATUS:
				outStr = statusPredToNLG(initiatorName, responderName, otherName);
				break;
			case EPredicateType::CKBENTRY:
				outStr = ckbPredToNLG(initiatorName, responderName, otherName);
				break;
			case EPredicateType::SFDB_LABEL:
				outStr = sfdbPredToNLG(initiatorName, responderName, otherName);
				break;
		}
	}
	else {
		if (mType == EPredicateType::CKBENTRY) {
			outStr = ckbPredToNLG(initiatorName, responderName, otherName);
		}
		else {
			outStr = numTimesUniquelyTruePredToNLG(initiatorName, responderName, otherName);
		}
	}

	// append the sfdb window information to the natural language predicate
	FString timeElapsedStr;
	if (mIsSFDB) {
		if (mWindowSize < 0) {
			// backstory
			timeElapsedStr = " way back when";
		}
		else if (mWindowSize >= 0 && mWindowSize <= 5) {
			timeElapsedStr = " recently";
		}
		else if (mWindowSize > 5 && mWindowSize <= 10) {
			timeElapsedStr = " not too long ago";
		}
		else {
			timeElapsedStr = " a while ago";
		}
	}

	outStr += timeElapsedStr;

	// if we are dealing with something with an SFDB order, we just kind of append that ot the end
	if (mSFDBOrder > 0) {
		outStr += " " + sfdbOrderToNLG();
	}

	return outStr;
}

FString UCiFPredicate::traitPredToNLG(const FName initiatorName, const FName responderName, const FName otherName)
{
	FString outStr;
	FString first = getRoleValue(mPrimary).ToString();
	if (first == "initiator") {
		outStr = mPrimary.ToString();
	}
	else if (first == "responder") {
		outStr = mSecondary.ToString();
	}
	else {
		outStr = mTertiary.ToString();
	}

	FString notStr = mIsNegated ? " not " : " ";

	auto traitEnum = StaticEnum<ETrait>();

	outStr += " is" + notStr + traitEnum->GetValueAsString(mTrait);
	return outStr;
}

FString UCiFPredicate::relationshipPredToNLG(const FName initiatorName, const FName responderName, const FName otherName) const
{
	FString notStr = mIsNegated ? " not " : " ";
	FString outStr;
	switch (mRelationshipType) {
		case ERelationshipType::FRIENDS:
			outStr = initiatorName.ToString() + " is" + notStr + " friends with " + responderName.ToString();
			break;
		case ERelationshipType::DATING:
			outStr = initiatorName.ToString() + " is" + notStr + " dating " + responderName.ToString();
			break;
		case ERelationshipType::ENEMIES:
			outStr = initiatorName.ToString() + " and " + responderName.ToString() + " are " + notStr + "enemies";
			break;
		default:
			outStr = "unrecognized relationship type";
	}
	return outStr;
}

FString UCiFPredicate::networkPredToNLG(const FName initiatorName, const FName responderName, const FName otherName) const
{
	FString initiator, responder;

	if (getRoleValue(mPrimary) == "initiator") initiator = initiatorName.ToString();
	else if (getRoleValue(mPrimary) == "responder") initiator = responderName.ToString();
	else initiator = mTertiary.ToString();

	if (getRoleValue(mSecondary) == "initiator") responder = initiatorName.ToString();
	else if (getRoleValue(mSecondary) == "responder") responder = responderName.ToString();
	else responder = mTertiary.ToString();

	auto networkEnum = StaticEnum<ESocialNetworkType>();
	if (mComparatorType == EComparatorType::LESS_THAN) {
		if (mIsNegated) {
			return initiator + " " + networkEnum->GetValueAsString(mNetworkType) + " towards " + responder + " isn't low enough";
		}
		else {
			return initiator + " " + networkEnum->GetValueAsString(mNetworkType) + " towards " + responder + " is low enough";
		}
	}

	if (mComparatorType == EComparatorType::GREATER_THAN) {
		if (mIsNegated) {
			return initiator + " " + networkEnum->GetValueAsString(mNetworkType) + " towards " + responder + " isn't high enough";
		}
		else {
			return initiator + " " + networkEnum->GetValueAsString(mNetworkType) + " towards " + responder + " is high enough";
		}
	}

	return "lazy to implement all comparator types in network predicate to NLG";
}

FString UCiFPredicate::statusPredToNLG(const FName initiatorName, const FName responderName, const FName otherName) const
{
	FString outStr;
	FString towardsName;

	auto first = getRoleValue(mPrimary);
	if (first == "initiator") outStr = initiatorName.ToString();
	else if (first == "responder") outStr = responderName.ToString();
	else outStr = otherName.ToString();

	auto second = getRoleValue(mSecondary);
	if (second == "initiator") towardsName = initiatorName.ToString();
	else if (second == "responder") towardsName = responderName.ToString();
	else towardsName = otherName.ToString();

	FString notStr = mIsNegated ? " not " : " ";

	auto statusEnum = StaticEnum<EStatus>();
	outStr += " is" + notStr + statusEnum->GetValueAsString(mStatusType);
	if (mStatusType >= EStatus::FIRST_DIRECTED_STATUS) {
		outStr += " towards " + towardsName;
	}

	return outStr;
}

FString UCiFPredicate::ckbPredToNLG(const FName initiatorName, const FName responderName, const FName otherName) const
{
	FString negatedStr;
	if (mIsNegated) {
		negatedStr = "it isn't the case that ";
	}

	auto truthEnum = StaticEnum<ETruthLabel>();
	FString gestaltTruth = truthEnum->GetValueAsString(mTruthLabel);

	FString amountOfThings;
	if (mNumTimesUniquelyTrue == 1 || mNumTimesUniquelyTrue == 0) amountOfThings = "something " + gestaltTruth;
	if (mNumTimesUniquelyTrue == 2) amountOfThings = "a couple " + gestaltTruth + " things";
	if (mNumTimesUniquelyTrue >= 3) amountOfThings = "a lot of " + gestaltTruth + " things";

	auto subjLabelEnum = StaticEnum<ESubjectiveLabel>();
	FString outStr;
	if (mFirstSubjectiveLink != mSecondSubjectiveLink) {
		// primary and secondary disagree
		//based on the num times uniquely true, it will use a different word to describe it!
		//But where do I handle negation?
		outStr = negatedStr + initiatorName.ToString() + subjLabelEnum->GetValueAsString(mFirstSubjectiveLink) + " " +
			amountOfThings + " that " + responderName.ToString() + subjLabelEnum->GetValueAsString(mSecondSubjectiveLink);
	}
	else {
		outStr = initiatorName.ToString() + " and " + responderName.ToString() + " both " +
			subjLabelEnum->GetValueAsString(mFirstSubjectiveLink) + " " + amountOfThings;
	}

	return outStr;
}

FString UCiFPredicate::sfdbPredToNLG(const FName initiatorName, const FName responderName, const FName otherName)
{
	FString sfdbLabelType;
	if (mSFDBLabel.type == ESFDBLabelType::CAT_POSITIVE) sfdbLabelType = "generally positive";
	if (mSFDBLabel.type == ESFDBLabelType::CAT_NEGATIVE) sfdbLabelType = "generally negative";

	FString outStr = initiatorName.ToString() + " did something " + sfdbLabelType + " to " + responderName.ToString();
	if (initiatorName == responderName || responderName == "") {
		// this is a case where the SFDB should only care about the primary person
		outStr = initiatorName.ToString() + " did something " + sfdbLabelType;
	}
	return outStr;
}

FString UCiFPredicate::numTimesUniquelyTruePredToNLG(const FName initiatorName, const FName responderName, const FName otherName)
{
	return "please god im not built for doing this right now";
	FString notStr = mIsNegated ? " not" : "";

	FString heroName;
	switch (mNumTimesRoleSlot) {
		case ENumTimesRoleSlot::FIRST: heroName = initiatorName.ToString();
			break;
		case ENumTimesRoleSlot::SECOND: heroName = responderName.ToString();
			break;
		case ENumTimesRoleSlot::BOTH: heroName = initiatorName.ToString() + " and " + responderName.ToString();
			break;
		default: heroName = "invalid role slot";
			break;
	}

	FString outStr;
	switch (mType) {
		case EPredicateType::TRAIT:
			UE_LOG(LogTemp, Error, TEXT("Trait can't be num times uniquely true"));
			break;
		case EPredicateType::NETWORK:
			{
				bool isHigh = false, isLow = false, isMed = false;
				if (mNetworkValue == 66 && mComparatorType == EComparatorType::GREATER_THAN) isHigh = true;
				else if (mNetworkValue == 34 && mComparatorType == EComparatorType::LESS_THAN) isLow = true;
				else if ((mNetworkValue == 33 && mComparatorType == EComparatorType::GREATER_THAN) ||
					(mNetworkValue == 67 && mComparatorType == EComparatorType::LESS_THAN))
					isMed = true;

				switch (mNumTimesRoleSlot) {
					case ENumTimesRoleSlot::FIRST:
						{
							switch (mNetworkType) {
								case ESocialNetworkType::BUDDY:
									if (isLow) {
										if (mIsNegated) {
											outStr = heroName + " does not even think that at least " + mNumTimesUniquelyTrue +
												" people aren't very good buddies.";
										}
										else {
											outStr = heroName + " thinks at least " + mNumTimesUniquelyTrue +
												" people aren't very good buddies.";
										}
									}
									else if (isMed) {
										if (mIsNegated) {
											outStr = heroName + " does not even think that at least " + mNumTimesUniquelyTrue +
												" people are OK buddies.";
										}
										else {
											outStr = heroName + " thinks at least " + mNumTimesUniquelyTrue + " people are OK buddies.";
										}
									}
									else if (isHigh) {
										if (mIsNegated) {
											outStr = heroName + " does not even think that at least " + mNumTimesUniquelyTrue +
												" people are great buddies.";
											break;
										}
										else {
											outStr = heroName + " thinks at least " + mNumTimesUniquelyTrue + " people are great buddies.";
											break;
										}
									}
									else {
										outStr = "problem with numTimesUniqelyTrue network predicate to Natural Language";
										break;
									}
									break;
								case ESocialNetworkType::ROMANCE:
									if (isLow) {
										if (mIsNegated) {
											outStr = heroName + " isn't completely romantically disinterested in at least " +
												mNumTimesUniquelyTrue + " people.";
											break;
										}
										else {
											outStr = heroName + " is romantically turned off by at least " + mNumTimesUniquelyTrue +
												" people.";
											break;
										}
									}
									else if (isMed) {
										if (mIsNegated) {
											outStr = heroName + " isn't kinda romantically interested in at least " + mNumTimesUniquelyTrue
												+ " people.";
										}
										else {
											outStr = heroName + " is kinda romantically interested in at least " + mNumTimesUniquelyTrue +
												" people.";
										}
									}
									else if (isHigh) {
										if (mIsNegated) {
											outStr = heroName + " is not head over heels in love with at least " + mNumTimesUniquelyTrue +
												" people.";
										}
										else {
											outStr = heroName + " is head over heels gaga for at least" + mNumTimesUniquelyTrue +
												" people.";
										}
									}
									else {
										outStr = "problem with numTimesUniqelyTrue network predicate to Natural Language";
									}
									break;
								case ESocialNetworkType::TRUST:
									if (isLow) {
										if (mIsNegated) {
											outStr = heroName + " does not think that at least " + mNumTimesUniquelyTrue +
												" people are pretty darn lame.";
										}
										else {
											outStr = heroName + " thinks at least " + mNumTimesUniquelyTrue +
												" people are pretty darn lame.";
										}
									}
									else if (isMed) {
										if (mIsNegated) {
											outStr = heroName + " does not think that at least " + mNumTimesUniquelyTrue +
												" people are actually kinda cool.";
										}
										else {
											outStr = heroName + " thinks at least " + mNumTimesUniquelyTrue +
												" people are actually kinda cool.";
										}
									}
									else if (isHigh) {
										if (mIsNegated) {
											outStr = heroName + " does not think that at least " + mNumTimesUniquelyTrue +
												" people are wicked cool.";
										}
										else {
											outStr = heroName + " thinks at least " + mNumTimesUniquelyTrue + " people are wicked cool.";
										}
									}
									else {
										outStr = "problem with numTimesUniqelyTrue network predicate to Natural Language";
									}
									break;
								case ESocialNetworkType::SIZE:
									break;
							}
						}
						break;
					case ENumTimesRoleSlot::SECOND:
						break;
				}

				break;
			}
		case EPredicateType::RELATIONSHIP:
			break;
		case EPredicateType::STATUS:
			break;
		case EPredicateType::CKBENTRY:
			break;
		case EPredicateType::SFDB_LABEL:
			break;
		case EPredicateType::SIZE:
			break;
	}

	return outStr;
}

FString UCiFPredicate::sfdbOrderToNLG()
{
	if (mSFDBOrder == 1) return "first";
	if (mSFDBOrder == 2) return "second";
	if (mSFDBOrder == 3) return "third";
	if (mSFDBOrder == 4) return "fourth";
	if (mSFDBOrder == 5) return "fifth";
	if (mSFDBOrder == 6) return "sixth";
	if (mSFDBOrder == 7) return "seventh";
	if (mSFDBOrder == 8) return "eighth";
	if (mSFDBOrder == 9) return "ninth";
	return "how did we get here";
}

void UCiFPredicate::toString(FString& outStr) const
{
	if (mIsNegated) outStr += "~";

	switch (mType) {
		case EPredicateType::TRAIT:
			{
				auto traitEnum = StaticEnum<ETrait>();
				outStr += "trait(" + mPrimary.ToString() + ", " + traitEnum->GetValueAsString(mTrait) + ")";
				break;
			}
		case EPredicateType::NETWORK:
			{
				auto socialNetEnum = StaticEnum<ESocialNetworkType>();
				auto comparatorEnum = StaticEnum<EComparatorType>();
				outStr += socialNetEnum->GetValueAsString(mNetworkType) + " Network(" + mPrimary.ToString() + ", " + mSecondary.ToString() +
					") " + comparatorEnum->GetValueAsString(mComparatorType) + " " + FString::FromInt(mNetworkValue);
				break;
			}
		case EPredicateType::RELATIONSHIP:
			{
				auto relationshipEnum = StaticEnum<ERelationshipType>();
				outStr += "relationship(" + mPrimary.ToString() + ", " + mSecondary.ToString() + relationshipEnum->
					GetValueAsString(mRelationshipType) + ")";
				break;
			}
		case EPredicateType::STATUS:
			{
				auto statusEnum = StaticEnum<EStatus>();
				if (mStatusType >= EStatus::FIRST_DIRECTED_STATUS || mStatusType == EStatus::CAT_FEELING_GOOD_ABOUT_SOMEONE ||
					mStatusType == EStatus::CAT_FEELING_BAD_ABOUT_SOMEONE) {
					outStr += "status(" + mPrimary.ToString() + ", " + mSecondary.ToString() + ", " + statusEnum->
						GetValueAsString(mStatusType)
						+ ", " + FString::FromInt(mStatusDuration) + ")";
				}
				else {
					outStr += "status(" + mPrimary.ToString() + ", " + statusEnum->GetValueAsString(mStatusType)
						+ ", " + FString::FromInt(mStatusDuration) + ")";
				}
				break;
			}
		case EPredicateType::CKBENTRY:
			{
				auto subjLinkEnum = StaticEnum<ESubjectiveLabel>();
				auto truthLabelEnum = StaticEnum<ETruthLabel>();
				outStr += "ckb(" + mPrimary.ToString() + ", " + subjLinkEnum->GetValueAsString(mFirstSubjectiveLink) + ", " + mSecondary.
					ToString() + ", " + subjLinkEnum->GetValueAsString(mSecondSubjectiveLink) + ", " + truthLabelEnum->
					GetValueAsString(mTruthLabel) + ")";
				break;
			}
		case EPredicateType::SFDB_LABEL:
			{
				auto sfdbLabelEnum = StaticEnum<ESFDBLabelType>();
				outStr += "SFDBLabel(" + sfdbLabelEnum->GetValueAsString(mSFDBLabel.type) + "," + mPrimary.ToString() + "," + mSecondary.
					ToString() + "," + FString::FromInt(mSFDBOrder) + ")";
				break;
			}
	}
	if (mIsSFDB) outStr = "[" + outStr + " window(" + FString::FromInt(mWindowSize) + ")]";
	if (mIsIntent) outStr = "{" + outStr + "}";
}

bool UCiFPredicate::operator==(const UCiFPredicate& other) const
{
	if (mType != other.mType) return false;
	if (mIsIntent != other.mIsIntent) return false;
	if (mIsNegated != other.mIsNegated) return false;
	if (mIsSFDB != other.mIsSFDB) return false;
	if (mIsNumTimesUniquelyTruePred != other.mIsNumTimesUniquelyTruePred) return false;
	if (mNumTimesUniquelyTrue != other.mNumTimesUniquelyTrue) return false;
	if (mIsNumTimesUniquelyTruePred) {
		if (mNumTimesRoleSlot != other.mNumTimesRoleSlot) return false;
	}

	switch (mType) {
		case EPredicateType::TRAIT:
			if (mTrait != other.mTrait) return false;
			if (mPrimary != other.mPrimary) return false;
			break;
		case EPredicateType::NETWORK:
			if ((mPrimary != other.mPrimary) ||
				(mSecondary != other.mSecondary) ||
				(mNetworkType != other.mNetworkType) ||
				(mNetworkValue != other.mNetworkValue) ||
				(mComparatorType != other.mComparatorType))
				return false;
			break;
		case EPredicateType::RELATIONSHIP:
			if ((mRelationshipType != other.mRelationshipType) ||
				!(mPrimary == other.mPrimary && mSecondary == other.mSecondary) ||
				!(mSecondary == other.mPrimary && mPrimary == other.mSecondary))
				return false;
			break;
		case EPredicateType::STATUS:
			if ((mStatusType != other.mStatusType) ||
				(mPrimary != other.mPrimary) ||
				(mSecondary != other.mSecondary))
				return false;
			break;
		case EPredicateType::CKBENTRY:
			if ((mPrimary != other.mPrimary) ||
				(mSecondary != other.mSecondary) ||
				(mFirstSubjectiveLink != other.mFirstSubjectiveLink) ||
				(mSecondSubjectiveLink != other.mSecondSubjectiveLink) ||
				(mTruthLabel != other.mTruthLabel))
				return false;
			break;
		case EPredicateType::SFDB_LABEL:
			if ((mPrimary != other.mPrimary) ||
				(mSecondary != other.mSecondary) ||
				(mSFDBOrder != other.mSFDBOrder) ||
				(mSFDBLabel != other.mSFDBLabel))
				return false;
			break;
	}

	return true;
}

FName UCiFPredicate::getValueOfPredicateVariable(const FName var) const
{
	if (var == "i" || var == "initiator") return "initiator";
	if (var == "r" || var == "responder") return "responder";
	if (var == "o" || var == "other") return "other";

	return "";
}

ERelationshipType UCiFPredicate::comparatorTypeToRelationshipType(const EComparatorType comparatorType) const
{
	switch (comparatorType) {
		case EComparatorType::FRIENDS_OPINION: return ERelationshipType::FRIENDS;
		case EComparatorType::DATING_OPINION: return ERelationshipType::DATING;
		case EComparatorType::ENEMIES_OPINION: return ERelationshipType::ENEMIES;
		default:
			UE_LOG(LogTemp, Warning, TEXT("Compartor type %d is not of a relationship networks"), comparatorType);
			return ERelationshipType::FRIENDS; // return this value as a default
	}
}

bool UCiFPredicate::isSFDBLabelCategory() const
{
	return (mType == EPredicateType::SFDB_LABEL) && (mSFDBLabel.type <= ESFDBLabelType::CAT_LAST);
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

UCiFPredicate* UCiFPredicate::loadFromJson(TSharedPtr<FJsonObject> predJson, const UObject* worldContextObject)
{
	auto p = NewObject<UCiFPredicate>(const_cast<UObject*>(worldContextObject));

	const UEnum* predicateEnum = StaticEnum<EPredicateType>();
	p->mType = static_cast<EPredicateType>(predicateEnum->GetValueByName(FName(predJson->GetStringField("_type"))));
	p->mName = FName(predJson->GetStringField("_name"));
	UE_LOG(LogTemp, Log, TEXT("Parsing predicate: %s"), *(p->mName.ToString()));
	auto isSFDB = false;
	auto isNegated = false;

	switch (p->mType) {
		case EPredicateType::INVALID:
			break;
		case EPredicateType::TRAIT:
			{
				const UEnum* traitEnum = StaticEnum<ETrait>();
				const auto trait = static_cast<ETrait>(traitEnum->GetValueByName(FName(predJson->GetStringField("_trait"))));
				isSFDB = predJson->GetBoolField("_isSFDB");
				isNegated = predJson->GetBoolField("_negated");
				const auto first = FName(predJson->GetStringField("_first"));
				p->setTraitPredicate(first, trait, isNegated, isSFDB);
				break;
			}
		case EPredicateType::NETWORK:
			{
				isSFDB = predJson->GetBoolField("_isSFDB");
				isNegated = predJson->GetBoolField("_negated");
				const auto first = FName(predJson->GetStringField("_first"));
				const auto second = FName(predJson->GetStringField("_second"));
				const UEnum* comparatorEnum = StaticEnum<EComparatorType>();
				const auto comparator = static_cast<EComparatorType>(comparatorEnum->
					GetValueByName(FName(predJson->GetStringField("_comparator"))));
				const auto netVal = predJson->GetNumberField("_value");
				const UEnum* netTypeEnum = StaticEnum<ESocialNetworkType>();
				const auto netType = static_cast<ESocialNetworkType>(netTypeEnum->
					GetValueByName(FName(predJson->GetStringField("_networkType"))));

				p->setNetworkPredicate(first, second, comparator, netVal, netType, isNegated, isSFDB);
				break;
			}
		case EPredicateType::RELATIONSHIP:
			{
				isSFDB = predJson->GetBoolField("_isSFDB");
				isNegated = predJson->GetBoolField("_negated");
				const auto first = FName(predJson->GetStringField("_first"));
				const auto second = FName(predJson->GetStringField("_second"));
				const UEnum* relTypeEnum = StaticEnum<ERelationshipType>();
				const auto relType = static_cast<ERelationshipType>(relTypeEnum->
					GetValueByName(FName(predJson->GetStringField("_relationship"))));

				p->setRelationshipPredicate(first, second, relType, isNegated, isSFDB);
			}
			break;
		case EPredicateType::STATUS:
			{
				isSFDB = predJson->GetBoolField("_isSFDB");
				isNegated = predJson->GetBoolField("_negated");
				const auto first = FName(predJson->GetStringField("_first"));
				const auto second = FName(predJson->GetStringField("_second"));
				const UEnum* statusEnum = StaticEnum<EStatus>();
				const auto status = static_cast<EStatus>(statusEnum->
					GetValueByName(FName(predJson->GetStringField("_status"))));

				int32 duration = 0;
				predJson->TryGetNumberField("_duration", duration);

				p->setStatusPredicate(first, second, status, duration, isSFDB, isNegated);
			}
			break;
		case EPredicateType::CKBENTRY:
			{
				isNegated = predJson->GetBoolField("_negated");
				const auto first = FName(predJson->GetStringField("_first"));
				const auto second = FName(predJson->GetStringField("_second"));

				const auto connectionTypeEnum = StaticEnum<ESubjectiveLabel>();
				auto connectionTypeName = FName(predJson->GetStringField("_firstSubjective"));
				const auto firstSubjective = connectionTypeName == ""
					                             ? ESubjectiveLabel::INVALID
					                             : static_cast<ESubjectiveLabel>(connectionTypeEnum->GetValueByName(connectionTypeName));

				connectionTypeName = FName(predJson->GetStringField("_secondSubjective"));
				const auto secondSubjective = connectionTypeName == ""
					                              ? ESubjectiveLabel::INVALID
					                              : static_cast<ESubjectiveLabel>(connectionTypeEnum->GetValueByName(connectionTypeName));

				const auto truthEnum = StaticEnum<ETruthLabel>();
				const auto labelName = FName(predJson->GetStringField("_label"));
				const auto label = labelName == "" ? ETruthLabel::INVALID : static_cast<ETruthLabel>(truthEnum->GetValueByName(labelName));
				p->setCKBPredicate(first, second, firstSubjective, secondSubjective, label, isNegated);
			}
			break;
		case EPredicateType::SFDB_LABEL:
			{
				isNegated = predJson->GetBoolField("_negated");
				const auto first = FName(predJson->GetStringField("_first"));
				const auto second = FName(predJson->GetStringField("_second"));
				const UEnum* sfdbLabelEnum = StaticEnum<ESFDBLabelType>();
				const auto sfdbLabel = static_cast<ESFDBLabelType>(sfdbLabelEnum->
					GetValueByName(FName(predJson->GetStringField("_label"))));
				const auto window = predJson->GetNumberField("_window");
				p->setSFDBLabelPredicate(first, second, sfdbLabel, window, isNegated);
			}
			break;
		case EPredicateType::SIZE:
			break;
	}

	p->mIsIntent = predJson->GetBoolField("_intent");
	if (p->mIsIntent) {
		const UEnum* intentTypeEnum = StaticEnum<EIntentType>();
		p->mIntentType = static_cast<EIntentType>(intentTypeEnum->GetValueByName(FName(predJson->GetStringField("_intentType"))));
	}

	p->mIsNumTimesUniquelyTruePred = predJson->GetBoolField("_numTimesUniquelyTrueFlag");
	p->mNumTimesUniquelyTrue = 0;
	if (p->mIsNumTimesUniquelyTruePred) {
		p->mNumTimesUniquelyTrue = predJson->GetNumberField("_numTimesUniquelyTrue");
		const UEnum* roleSlotEnum = StaticEnum<ENumTimesRoleSlot>();
		p->mNumTimesRoleSlot = static_cast<ENumTimesRoleSlot>(roleSlotEnum->
			GetValueByName(FName(predJson->GetStringField("_numTimesRoleSlot"))));
	}

	p->mSFDBOrder = 0;
	predJson->TryGetNumberField("_sfdbOrder", p->mSFDBOrder);

	return p;
}
