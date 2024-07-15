// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFManager.h"
#include "CiFCast.h"
#include "CiFCharacter.h"
#include "CiFCulturalKnowledgeBase.h"
#include "CiFInstantiation.h"
#include "CiFItem.h"
#include "CiFKnowledge.h"
#include "CiFMicrotheory.h"
#include "CiFPredicate.h"
#include "CiFProspectiveMemory.h"
#include "CiFRelationshipNetwork.h"
#include "CiFRule.h"
#include "CiFSocialExchange.h"
#include "CiFSocialExchangeContext.h"
#include "CiFSocialExchangesLibrary.h"
#include "CiFStatusContext.h"
#include "CiFTrigger.h"
#include "CiFTriggerContext.h"
#include "ReadWriteFiles.h"

UCiFManager::UCiFManager()
{
	mTime = 0;
}

void UCiFManager::init(const UObject* worldContextObject)
{
	mSocialExchangesLib = NewObject<UCiFSocialExchangesLibrary>(const_cast<UObject*>(worldContextObject));
	mSFDB = NewObject<UCiFSocialFactsDataBase>(const_cast<UObject*>(worldContextObject));
	// TODO - its not correct to put it here. it should happen on init but on "start game" or something, because if the
	// player has already has save game, we need to just load it from the save game, although it should be the same data.
	// for now i'll put it here

	const FString sgLibPath = FPaths::Combine(*FPaths::ProjectPluginsDir(), *FString("CiF/Content/Data/socialGameLib.json"));
	UE_LOG(LogTemp, Log, TEXT("Reading social games from %s"), *sgLibPath);
	loadSocialGameLib(sgLibPath, worldContextObject);

	const FString mtLibPath = FPaths::Combine(*FPaths::ProjectPluginsDir(), *FString("CiF/Content/Data/microtheories.json"));
	UE_LOG(LogTemp, Log, TEXT("Reading microtheories from %s"), *mtLibPath);
	loadMicrotheories(mtLibPath, worldContextObject);

	const FString castPath = FPaths::Combine(*FPaths::ProjectPluginsDir(), *FString("CiF/Content/Data/cast.json"));
	UE_LOG(LogTemp, Log, TEXT("Reading cast from %s"), *castPath);
	loadCast(castPath, worldContextObject);

	const FString itemsPath = FPaths::Combine(*FPaths::ProjectPluginsDir(), *FString("CiF/Content/Data/items.json"));
	UE_LOG(LogTemp, Log, TEXT("Reading items from %s"), *itemsPath);
	loadItemList(itemsPath, worldContextObject);

	const FString knowledgePath = FPaths::Combine(*FPaths::ProjectPluginsDir(), *FString("CiF/Content/Data/knowledgeList.json"));
	UE_LOG(LogTemp, Log, TEXT("Reading knowledge list from %s"), *knowledgePath);
	loadKnowledgeList(knowledgePath, worldContextObject);

	const FString sfdbPath = FPaths::Combine(*FPaths::ProjectPluginsDir(), *FString("CiF/Content/Data/sfdb.json"));
	UE_LOG(LogTemp, Log, TEXT("Reading SFDB from %s"), *sfdbPath);
	loadSFDB(sfdbPath, worldContextObject);

	const FString triggersPath = FPaths::Combine(*FPaths::ProjectPluginsDir(), *FString("CiF/Content/Data/triggers.json"));
	UE_LOG(LogTemp, Log, TEXT("Reading triggers from %s"), *triggersPath);
	loadTriggers(triggersPath, worldContextObject);

	const FString socialNetworksPath = FPaths::Combine(*FPaths::ProjectPluginsDir(), *FString("CiF/Content/Data/socialNetworks.json"));
	UE_LOG(LogTemp, Log, TEXT("Reading social networks from %s"), *socialNetworksPath);
	loadSocialNetworks(socialNetworksPath, worldContextObject);

	const FString ckbPath = FPaths::Combine(*FPaths::ProjectPluginsDir(), *FString("CiF/Content/Data/ckb.json"));
	UE_LOG(LogTemp, Log, TEXT("Reading CKB from %s"), *ckbPath);
	loadCKB(ckbPath, worldContextObject);

	UE_LOG(LogTemp, Log, TEXT("Finished loading all"));
}

void UCiFManager::loadSocialGameLib(const FString& filePath, const UObject* worldContextObject)
{
	mSocialExchangesLib->loadSocialGamesLibFromJson(filePath, worldContextObject);
}

void UCiFManager::loadMicrotheories(const FString& filePath, const UObject* worldContextObject)
{
	TSharedPtr<FJsonObject> jsonObject;
	if (!UReadWriteFiles::readJson(filePath, jsonObject)) {
		return;
	}

	const auto microtheoriesJson = jsonObject->GetArrayField("Microtheories");
	for (const auto mtJson : microtheoriesJson) {
		auto mt = UCiFMicrotheory::loadFromJson(mtJson->AsObject(), worldContextObject);
		mMicrotheoriesLib.Add(mt->mName, mt);
	}
}

void UCiFManager::loadCast(const FString& filePath, const UObject* worldContextObject)
{
	TSharedPtr<FJsonObject> jsonObject;
	if (!UReadWriteFiles::readJson(filePath, jsonObject)) {
		return;
	}

	mCast = UCiFCast::loadFromJson(jsonObject, worldContextObject);
}

void UCiFManager::loadItemList(const FString& filePath, const UObject* worldContextObject)
{
	TSharedPtr<FJsonObject> jsonObject;
	if (!UReadWriteFiles::readJson(filePath, jsonObject)) {
		return;
	}

	const auto itemsJson = jsonObject->GetArrayField("Items");
	for (const auto itemJson : itemsJson) {
		auto item = UCiFItem::loadFromJson(itemJson->AsObject(), worldContextObject);
		mItemArray.Add(item);
	}
}

void UCiFManager::loadKnowledgeList(const FString& filePath, const UObject* worldContextObject)
{
	TSharedPtr<FJsonObject> jsonObject;
	if (!UReadWriteFiles::readJson(filePath, jsonObject)) {
		return;
	}

	const auto knowledgeJson = jsonObject->GetArrayField("Knowledge");
	for (const auto kJson : knowledgeJson) {
		auto knowledge = UCiFKnowledge::loadFromJson(kJson->AsObject(), worldContextObject);
		mKnowledgeArray.Add(knowledge);
	}
}

void UCiFManager::loadCKB(const FString& filePath, const UObject* worldContextObject)
{
	TSharedPtr<FJsonObject> jsonObject;
	if (!UReadWriteFiles::readJson(filePath, jsonObject)) {
		return;
	}

	mCKB = UCiFCulturalKnowledgeBase::loadFromJson(jsonObject, worldContextObject);
}

void UCiFManager::loadSFDB(const FString& filePath, const UObject* worldContextObject)
{
	TSharedPtr<FJsonObject> jsonObject;
	if (!UReadWriteFiles::readJson(filePath, jsonObject)) {
		return;
	}

	const auto scsJson = jsonObject->GetArrayField("StatusContext");
	for (const auto scJson : scsJson) {
		auto sc = UCiFStatusContext::loadFromJson(scJson->AsObject(), worldContextObject);
		if (sc) {
			mSFDB->mContexts.Add(sc);
		}
	}

	const auto tcsJson = jsonObject->GetArrayField("TriggerContext");
	for (const auto tcJson : tcsJson) {
		auto tc = UCiFTriggerContext::loadFromJson(tcJson->AsObject(), worldContextObject);
		if (tc) {
			mSFDB->mContexts.Add(tc);
		}
	}

	const auto sgcsJson = jsonObject->GetArrayField("SocialGameContext");
	for (const auto sgJson : sgcsJson) {
		auto sgc = UCiFSocialExchangeContext::loadFromJson(sgJson->AsObject(), worldContextObject);
		if (sgc) {
			mSFDB->mContexts.Add(sgc);
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("SocialGmaeContext failed to load from file"));
		}
	}

	const auto backstoryJson = jsonObject->GetArrayField("BackstoryContext");
	for (const auto bsJson : backstoryJson) {
		auto bsc = UCiFSocialExchangeContext::loadFromJson(bsJson->AsObject(), worldContextObject);
		if (bsc) {
			mSFDB->mContexts.Add(bsc);
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("SocialGmaeContext failed to load from file"));
		}
	}

	// sort the contexts in SFDB in the specified order (ascending in our case)
	// if want to sort in a descending order, need to provide lambda function that return a > b as true
	mSFDB->mContexts.Sort();
}

void UCiFManager::loadSocialNetworks(const FString& filePath, const UObject* worldContextObject)
{
	TSharedPtr<FJsonObject> jsonObject;
	if (!UReadWriteFiles::readJson(filePath, jsonObject)) {
		return;
	}

	const auto snsJson = jsonObject->GetArrayField("SocialNetworks");
	for (const auto snJson : snsJson) {
		auto sn = UCiFSocialNetwork::loadFromJson(snJson->AsObject(), worldContextObject);
		mSocialNetworks.Add(sn->mType, sn);
	}

	const auto rsJson = jsonObject->GetObjectField("RelationshipNetwork");
	mRelationshipNetworks = UCiFRelationshipNetwork::loadFromJson(rsJson, worldContextObject);
}

void UCiFManager::loadPlotPoints(const FString& filePath, const UObject* worldContextObject)
{
	// TODO - implement
}

void UCiFManager::loadQuestLib(const FString& filePath, const UObject* worldContextObject)
{
	// TODO - implement
}

void UCiFManager::loadTriggers(const FString& filePath, const UObject* worldContextObject)
{
	TSharedPtr<FJsonObject> jsonObject;
	if (!UReadWriteFiles::readJson(filePath, jsonObject)) {
		return;
	}

	const auto triggersJson = jsonObject->GetArrayField("Triggers");
	for (const auto triggerJson : triggersJson) {
		auto t = UCiFTrigger::loadFromJson(triggerJson->AsObject(), worldContextObject);
		if (t) {
			mSFDB->mTriggers.Add(t);
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("Trigger failed to load from file"));
		}
	}
}

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

void UCiFManager::formIntentForSocialGames(UCiFCharacter* initiator, UCiFGameObject* responder, const TArray<UCiFCharacter*>& possibleOthers)
{
	clearProspectiveMemory();

	for (auto [name, se] : mSocialExchangesLib->mSocialExchanges) {
		formIntentForSpecificSocialExchange(se, initiator, responder, possibleOthers);
	}
}

void UCiFManager::formIntentForSpecificSocialExchange(UCiFSocialExchange* socialExchange,
                                                      UCiFCharacter* initiator,
                                                      UCiFGameObject* responder,
                                                      const TArray<UCiFCharacter*>& possibleOthers)
{
	if (possibleOthers.Num() == 0) {
		auto calculatedPossibleOthers = socialExchange->getPossibleOthers(initiator, responder);
		formIntentThirdParty(socialExchange, initiator, responder, mCast->mCharacters);
	}
	else {
		formIntentThirdParty(socialExchange, initiator, responder, possibleOthers);
	}
}

void UCiFManager::formIntentThirdParty(UCiFSocialExchange* socialExchange,
                                       UCiFCharacter* initiator,
                                       UCiFGameObject* responder,
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
                                               UCiFGameObject* responder,
                                               const TArray<UCiFCharacter*>& possibleOthers)
{
	auto others = possibleOthers.Num() > 0 ? possibleOthers : mCast->mCharacters;
	int8 totalScore = 0;

	for (const auto [name, microTheory] : mMicrotheoriesLib) {
		totalScore += microTheory->score(initiator, responder, se, others);
	}

	return totalScore;
}

UCiFSocialExchangeContext* UCiFManager::playGame(UCiFSocialExchange* sg,
                                                 UCiFGameObject* initiator,
                                                 UCiFGameObject* responder,
                                                 UCiFGameObject* other,
                                                 TArray<UCiFGameObject*> otherCast,
                                                 TArray<UCiFGameObject*> levelCast,
                                                 UCiFEffect* chosenEffect)
{
	//The fact that other was ever passed in at all is an artifact of cif from days long gone.  
	//NOW we figure out who the other is in THIS function, when we call 'getSalientOtherAndEffect'
	//Since other part of this function (playGame) depend on other being null, we are going
	//to just set it to null here explicitly (since passing in, say, an instantiated yet 'blank' character with no name
	//will cause issues and heartbreak).
	other = nullptr;

	if (levelCast.IsEmpty()) {
		UE_LOG(LogTemp, Warning, TEXT("Level cast is empty, this is not allowed - but why?"));
	}
	TArray<UCiFGameObject*> possibleOthers = otherCast.IsEmpty() ? sg->getPossibleOthers(initiator, responder) : otherCast;

	const float score = getResponderScore(sg, initiator, responder, possibleOthers);

	const bool isAcceptGameIntent = (score >= 0) ? true : false;

	UCiFGameObject* mostSalientOther = nullptr;
	UCiFEffect* mostSalientEffect = nullptr;
	if (!chosenEffect) {
		getSalientOtherAndEffect(mostSalientOther,
		                         mostSalientEffect,
		                         sg,
		                         isAcceptGameIntent,
		                         initiator,
		                         responder,
		                         possibleOthers,
		                         levelCast);
	}
	else if (isAcceptGameIntent) {
		// Hack to pick a chosen Effect and other along with it
		mostSalientEffect = chosenEffect;
		// if there is 1 possible other, its because its been previously selected by the player
		// else shouldn't need an other, the effect should have been found on its own
		mostSalientOther = (possibleOthers.Num() == 1) ? possibleOthers[0] : nullptr;
	}
	else {
		// find either matching chosen effect rejection
		if (chosenEffect->mRejectId == -1) {
			// if there is no matching reject, find whatever
			getSalientOtherAndEffect(mostSalientOther,
			                         mostSalientEffect,
			                         sg,
			                         isAcceptGameIntent,
			                         initiator,
			                         responder,
			                         possibleOthers,
			                         levelCast);
		}
		else {
			// else pick the reject effect and the other similarly to accept
			mostSalientEffect = sg->getEffectById(chosenEffect->mId);
			mostSalientOther = (possibleOthers.Num() == 1) ? possibleOthers[0] : nullptr;
		}
	}

	if (!mostSalientEffect) {
		UE_LOG(LogTemp, Error, TEXT("This shouldn't happen. Didn't find effect for a social game, meaning it is meaningless SG"));
		return nullptr;
	}

	// the other to use when all cases of other being passed in a third character being needed when one is not provided
	UCiFGameObject* trueOther = (!mostSalientOther && sg->isThirdForSocialExchangePlay()) ? mostSalientOther : other;

	//TODO: sort of a hack. I want this in GameEngine... this is part of separating playGame and changeSocialState
	mLastResponderOther = trueOther;

	/* Preparing social game context for output */
	auto socialGameContext = NewObject<UCiFSocialExchangeContext>();

	if (mostSalientEffect->hasCKBReference()) {
		socialGameContext->mChosenItemCKB = pickAGoodCKBObject(initiator, responder, mostSalientEffect->getCKBReferencePredicate());
	}

	socialGameContext->mGameName = sg->mName;
	socialGameContext->mInitiatorName = initiator->mObjectName;
	socialGameContext->mResponderName = responder->mObjectName;

	if (mostSalientEffect->hasSFDBLabel()) {
		for (const auto p : mostSalientEffect->mChange->mPredicates) {
			if (p->mType == EPredicateType::SFDB_LABEL) {
				FSFDBLabel label;
				label.to = p->getSecondaryCharacterNameFromVariables(initiator, responder, other);
				label.from = p->getPrimaryCharacterNameFromVariables(initiator, responder, other);
				label.type = p->mSFDBLabel.type;
				socialGameContext->mSFDBLabels.Add(label);
			}
		}
	}

	socialGameContext->mOtherName = trueOther ? trueOther->mObjectName : "";
	socialGameContext->mTime = mTime;
	if (initiator->mGameObjectType == ECiFGameObjectType::CHARACTER) {
	}
	else {
		socialGameContext->mInitiatorScore = 0;
	}
	socialGameContext->mResponderScore = score;

	return socialGameContext;
}

float UCiFManager::getResponderScore(UCiFSocialExchange* sg,
                                     UCiFGameObject* initiator,
                                     UCiFGameObject* responder,
                                     const TArray<UCiFGameObject*>& activeOtherCast)
{
	const TArray<UCiFGameObject*> possibleOthers =
		activeOtherCast.IsEmpty() ? sg->getPossibleOthers(initiator, responder) : activeOtherCast;

	float score = sg->scoreSocialExchange(static_cast<UCiFCharacter*>(initiator), responder, possibleOthers, true);

	// score MT - look up responder's intent to play social game with initiator
	if (responder->mGameObjectType == ECiFGameObjectType::CHARACTER) {
		const auto r = static_cast<UCiFCharacter*>(responder);
		if (r->mProspectiveMemory->mIntentScoreCache[initiator->mNetworkId][static_cast<uint8>(sg->mIntents[0]->mPredicates[0]->
			getIntentType())] != r->mProspectiveMemory->getDefaultIntentScore()) {
			score += r->mProspectiveMemory->mIntentScoreCache[initiator->mNetworkId][static_cast<uint8>(sg->mIntents[0]->mPredicates[0]->
				getIntentType())];
		}
	}

	return score;
}

void UCiFManager::getSalientOtherAndEffect(UCiFGameObject*& outOther,
                                           UCiFEffect*& outEffect,
                                           UCiFSocialExchange* sg,
                                           const bool isSgAccepted,
                                           UCiFGameObject* initiator,
                                           UCiFGameObject* responder,
                                           const TArray<UCiFGameObject*>& otherCast,
                                           TArray<UCiFGameObject*> levelCast)
{
	outOther = nullptr; // initialize to nullptr in case for some reason nothing is found in this method
	outEffect = nullptr;
	const auto possibleOthers = otherCast.IsEmpty() ? sg->getPossibleOthers(initiator, responder) : otherCast;

	if (levelCast.IsEmpty()) {
		levelCast = possibleOthers;
	}

	TArray<UCiFEffect*> possibleSalientEffects;
	TArray<UCiFGameObject*> possibleSalientOthers;

	// find all valid effects (go through all others)
	for (const auto effect : sg->mEffects) {
		if (effect->mIsAccept == isSgAccepted) {
			if (sg->mIsRequiresOther) {
				for (const auto o : possibleOthers) {
					if ((o->mObjectName != initiator->mObjectName) && (o->mObjectName != responder->mObjectName)) {
						bool isCastMemberPresentInArea = false;
						// make sure the character is present in the area if the instantiation requires him to be
						const auto instantiation = sg->getInstantiationById(effect->mInstantiationId);
						if (instantiation->requiresOtherToPerform()) {
							if (levelCast.Contains(o)) {
								isCastMemberPresentInArea = true;
							}
						}
						else {
							isCastMemberPresentInArea = true;
						}

						//if we have passed the check that the character is in the level (or it doesn't matter if they are or not)
						// TODO - continue
						if (isCastMemberPresentInArea) {
							// check to see if this i,r,o group satisfied the condition
							if (effect->mCondition->evaluate(static_cast<UCiFCharacter*>(initiator), responder, o, sg) &&
								sg->mOtherType == o->mGameObjectType) {
								possibleSalientEffects.Add(effect);
								possibleSalientOthers.Add(o);
							}
						}
					}
				}
			}
			else {
				if (effect->mCondition->evaluate(static_cast<UCiFCharacter*>(initiator), responder, nullptr, sg)) {
					possibleSalientEffects.Add(effect);
					possibleSalientOthers.Add(nullptr);
				}
			}
		}
	}

	//go through all valid effects and choose the ones that have the highest salience score
	//at this point, we know all effects and others are valid
	float maxSaliency = -9999;
	for (int i = 0; i < possibleSalientEffects.Num(); i++) {
		possibleSalientEffects[i]->scoreSalience();
		if (maxSaliency < possibleSalientEffects[i]->mSalienceScore) {
			maxSaliency = possibleSalientEffects[i]->mSalienceScore;
			outOther = possibleSalientOthers[i];
			outEffect = possibleSalientEffects[i];
		}
	}
}

void UCiFManager::getAllSalientEffects(TArray<UCiFEffect*> outEffects,
                                       UCiFSocialExchange* sg,
                                       const bool isAccepted,
                                       UCiFGameObject* initiator,
                                       UCiFGameObject* responder,
                                       const TArray<UCiFGameObject*> otherCast,
                                       TArray<UCiFGameObject*> levelCast)
{
	auto& possibleOthers = otherCast.IsEmpty() ? sg->getPossibleOthers(initiator, responder) : otherCast;
	if (levelCast.IsEmpty()) {
		levelCast = possibleOthers;
	}

	TArray<UCiFGameObject*> possibleSalientOthers;

	// find all valid effects, make sure to go through all others
	for (const auto e : sg->mEffects) {
		if (e->mIsAccept == isAccepted) {
			if (sg->mIsRequiresOther) {
				for (const auto c : possibleOthers) {
					bool castMemberPresent = false;
					if ((c->mObjectName != initiator->mObjectName) && (c->mObjectName == responder->mObjectName)) {
						//make sure the character is in the level if the instantiation requires him to be
						auto instantiation = sg->getInstantiationById(e->mInstantiationId);
						if (instantiation && instantiation->requiresOtherToPerform()) {
							// see if the other is in the level
							for (const auto castMember : levelCast) {
								if (castMember->mObjectName == c->mObjectName) {
									castMemberPresent = true;
								}
							}
						}
						else {
							castMemberPresent = true; // doesn't mean he is present but that we don't need him
						}

						//if we have passed the check that the character is in the level (or it doesn't matter if they are or not)
						if (castMemberPresent) {
							//check to see if this i,r,o group satisfies the condition
							if (e->mCondition->evaluate(static_cast<UCiFCharacter*>(initiator), responder, c, sg)) {
								outEffects.Add(e);
								possibleSalientOthers.Add(c);
							}
						}
					}
				}
			}
			else {
				// in this case we don't require other
				if (e->mCondition->evaluate(static_cast<UCiFCharacter*>(initiator), responder, nullptr, sg)) {
					outEffects.Add(e);
					possibleSalientOthers.Add(nullptr);
				}
			}
		}
	}
}

FName UCiFManager::pickAGoodCKBObject(const UCiFGameObject* initiator,
                                      const UCiFGameObject* responder,
                                      const UCiFPredicate* ckbPredicate) const
{
	TArray<FName> potentialCKBObjects;
	ckbPredicate->evalCKBEntryForObjects(initiator, responder, potentialCKBObjects);

	// pick random one for now
	const auto randIndex = FMath::RandRange(0, potentialCKBObjects.Num() - 1);
	return potentialCKBObjects[randIndex];
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
