// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFManager.h"
#include "CiFCast.h"
#include "CiFCharacter.h"
#include "CiFItem.h"
#include "CiFKnowledge.h"
#include "CiFPredicate.h"
#include "CiFProspectiveMemory.h"
#include "CiFRule.h"
#include "CiFSocialExchange.h"
#include "CiFSocialExchangesLibrary.h"

void UCiFManager::formIntentForAll()
{
	clearProspectiveMemory();

	for (auto c : mCast->mCharacters) {
		formIntent(c);
	}
}

void UCiFManager::formIntent(UCiFCharacter* initiator)
{
	for (auto responder : mCast->mCharacters) {
		if (responder->mObjectName != initiator->mObjectName) {
			formIntentForSocialGames(initiator, responder, mCast->mCharacters);
		}
	}
}

void UCiFManager::formIntentForSocialGames(UCiFCharacter* initiator, UCiFCharacter* responder, const TArray<UCiFCharacter*>& possibleOthers)
{
	for (auto [name, se] : mSocialExchangesLib->mSocialExchanges) {
		formIntentForSpecificSocialExchange(se, initiator, responder, possibleOthers);
	}
}

void UCiFManager::formIntentForSpecificSocialExchange(UCiFSocialExchange* socialExchange,
                                                      UCiFCharacter* initiator,
                                                      UCiFCharacter* responder,
                                                      const TArray<UCiFCharacter*>& possibleOthers)
{
	if (possibleOthers.Num() == 0) {
		/* this is suitable for Mismanor where they have other than characters,
		 * representation for knowledge and items - off for now, later we can complicate stuff */
		// auto calculatedPossibleOthers = socialExchange->getPossibleOthers(initiator, responder);
		formIntentThirdParty(socialExchange, initiator, responder, mCast->mCharacters);
	}
	else {
		formIntentThirdParty(socialExchange, initiator, responder, possibleOthers);
	}
}

void UCiFManager::formIntentThirdParty(UCiFSocialExchange* socialExchange,
                                       UCiFCharacter* initiator,
                                       UCiFCharacter* responder,
                                       const TArray<UCiFCharacter*>& possibleOthers)
{
	int8 score = 0;

	if (socialExchange->checkPreconditionsVariableOther(initiator, responder, TArray<UCiFGameObject*>(possibleOthers))) {
		score += socialExchange->scoreSocialExchange(initiator, responder, TArray<UCiFGameObject*>(possibleOthers));

		const auto intentType = socialExchange->mIntents[0]->mPredicates[0]->getIntentType();
		const auto intentIndex = static_cast<uint8>(intentType);
		if (initiator->mProspectiveMemory->mIntentScoreCache[responder->mNetworkId][intentIndex] ==
			initiator->mProspectiveMemory->getDefaultIntentScore()) {
			auto singleScore = scoreAllMicrotheoriesForType(socialExchange, initiator, responder, possibleOthers);
			initiator->mProspectiveMemory->cacheIntentScore(responder, intentType, singleScore);
			score += singleScore;
		}
		else {
			score += initiator->mProspectiveMemory->mIntentScoreCache[responder->mNetworkId][intentIndex];
		}
	}
	else {
		score = initiator->mProspectiveMemory->getDefaultIntentScore();
	}

	initiator->mProspectiveMemory->addSocialExchangeScore(socialExchange->mName,
	                                                      initiator->mObjectName,
	                                                      responder->mObjectName,
	                                                      "",
	                                                      score);
}

int8 UCiFManager::scoreAllMicrotheoriesForType(UCiFSocialExchange* se,
                                               UCiFCharacter* initiator,
                                               UCiFCharacter* responder,
                                               const TArray<UCiFCharacter*>& possibleOthers)
{
	auto others = possibleOthers.Num() > 0 ? possibleOthers : mCast->mCharacters;
	int8 totalScore = 0;

	for (const auto microTheory : mMicrotheories) {
		totalScore += microTheory->scoreMicrotheoryByType(initiator, responder, se, others);
	}

	return totalScore;
}

UCiFGameObject* UCiFManager::getGameObjectByName(const FName name) const
{
	UCiFGameObject* gameObject = nullptr;

	gameObject = mCast->getCharByName(name);
	if (!gameObject) {
		gameObject = getItemByName(name);
	}
	if (!gameObject) {
		gameObject = getKnowledgeByName(name);
	}

	return gameObject;
}

UCiFItem* UCiFManager::getItemByName(const FName name) const
{
	auto item = mItemArray.FindByPredicate([=](const UCiFItem* i) { return i->mObjectName == name; });
	if (item) return *item;
	return nullptr;
}

UCiFKnowledge* UCiFManager::getKnowledgeByName(const FName name) const
{
	auto knowledge = mKnowledgeArray.FindByPredicate([=](const UCiFKnowledge* k) {
		return k->mObjectName == name;
	});
	if (knowledge) return *knowledge;
	return nullptr;
}

UCiFSocialNetwork* UCiFManager::getSocialNetworkByType(const ESocialNetworkType type) const
{
	auto sn = mSocialNetworks.Find(type);
	if (sn) {
		return *sn;
	}
	return nullptr;
}

void UCiFManager::clearProspectiveMemory()
{
	for (auto c : mCast->mCharacters) {
		c->resetProspectiveMemory();
	}
}
