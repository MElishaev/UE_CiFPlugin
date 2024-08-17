// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFManager.h"
#include "CiFCast.h"
#include "CiFCharacter.h"
#include "CiFCulturalKnowledgeBase.h"
#include "CiFInfluenceRule.h"
#include "CiFInstantiation.h"
#include "CiFItem.h"
#include "CiFKnowledge.h"
#include "CiFMicrotheory.h"
#include "CiFPredicate.h"
#include "CiFProspectiveMemory.h"
#include "CiFRelationshipNetwork.h"
#include "CiFRule.h"
#include "CiFRuleRecord.h"
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
	mWorldContextObject = const_cast<UObject*>(worldContextObject);
	
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
	mCast->init(const_cast<UObject*>(worldContextObject));
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

	// Dont load backstory context for now, they include characters not available in game.
	// const auto backstoryJson = jsonObject->GetArrayField("BackstoryContext");
	// for (const auto bsJson : backstoryJson) {
	// 	auto bsc = UCiFSocialExchangeContext::loadFromJson(bsJson->AsObject(), worldContextObject);
	// 	if (bsc) {
	// 		mSFDB->mContexts.Add(bsc);
	// 	}
	// 	else {
	// 		UE_LOG(LogTemp, Warning, TEXT("SocialGmaeContext failed to load from file"));
	// 	}
	// }

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
			formIntentForSocialGames(initiator, responder, static_cast<TArray<UCiFGameObject*>>(mCast->mCharacters));
		}
	}
}

void UCiFManager::formIntentForSocialGames(UCiFCharacter* initiator,
                                           UCiFGameObject* responder,
                                           const TArray<UCiFGameObject*>& possibleOthers)
{
	clearProspectiveMemory();

	for (auto [name, se] : mSocialExchangesLib->mSocialExchanges) {
		formIntentForSpecificSocialExchange(se, initiator, responder, possibleOthers);
	}
}

void UCiFManager::formIntentForSpecificSocialExchange(UCiFSocialExchange* socialExchange,
                                                      UCiFCharacter* initiator,
                                                      UCiFGameObject* responder,
                                                      const TArray<UCiFGameObject*>& possibleOthers)
{
	if (possibleOthers.Num() == 0) {
		TArray<UCiFGameObject*> calculatedPossibleOthers;
		socialExchange->getPossibleOthers(calculatedPossibleOthers, initiator->mObjectName, responder->mObjectName);
		formIntentThirdParty(socialExchange, initiator, responder, calculatedPossibleOthers);
	}
	else {
		formIntentThirdParty(socialExchange, initiator, responder, possibleOthers);
	}
}

void UCiFManager::formIntentThirdParty(UCiFSocialExchange* socialExchange,
                                       UCiFCharacter* initiator,
                                       UCiFGameObject* responder,
                                       const TArray<UCiFGameObject*>& possibleOthers)
{
	int8 score = 0;

	if (socialExchange->checkPreconditionsVariableOther(initiator, responder, possibleOthers)) {
		score += socialExchange->scoreSocialExchange(initiator, responder, possibleOthers);

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
                                               const TArray<UCiFGameObject*>& possibleOthers)
{
	TArray<UCiFGameObject*> others = possibleOthers.Num() > 0 ? possibleOthers : static_cast<TArray<UCiFGameObject*>>(mCast->mCharacters);
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
	//Since other part of this function depend on other being null, we are going
	//to just set it to null here explicitly (since passing in, say, an instantiated yet 'blank' character with no name
	//will cause issues and heartbreak).
	other = nullptr;

	if (levelCast.IsEmpty()) {
		UE_LOG(LogTemp, Warning, TEXT("Level cast is empty, this is not allowed - but why?"));
	}
	
	TArray<UCiFGameObject*> possibleOthers = otherCast;
	if (possibleOthers.IsEmpty()) {
		sg->getPossibleOthers(possibleOthers, initiator->mObjectName, responder->mObjectName);
	}

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
		if (chosenEffect->mRejectId == CIF_INVALID_ID) {
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
	UCiFGameObject* trueOther = (!other && sg->isThirdForSocialExchangePlay()) ? mostSalientOther : other;

	//TODO: sort of a hack. I want this in GameEngine... this is part of separating playGame and changeSocialState
	mLastResponderOther = trueOther;

	/* Preparing social game context for output */
	const auto socialGameContext = NewObject<UCiFSocialExchangeContext>(mWorldContextObject);

	if (mostSalientEffect->hasCKBReference()) {
		socialGameContext->mChosenItemCKB = pickAGoodCKBObject(initiator, responder, mostSalientEffect->getCKBReferencePredicate());
	}

	socialGameContext->mGameName = sg->mName;
	socialGameContext->mEffectId = mostSalientEffect->mId;
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
	TArray<UCiFGameObject*> possibleOthers = activeOtherCast;
	if (possibleOthers.IsEmpty()) {
		sg->getPossibleOthers(possibleOthers, initiator->mObjectName, responder->mObjectName);
	}

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
	auto possibleOthers = otherCast;
	if (possibleOthers.IsEmpty()) {
		sg->getPossibleOthers(possibleOthers, initiator->mObjectName, responder->mObjectName);
	}

	if (levelCast.IsEmpty()) {
		levelCast = possibleOthers;
	}

	TArray<UCiFEffect*> possibleSalientEffects;
	TArray<UCiFGameObject*> possibleSalientOthers;

	// find all valid effects (go through all others)
	for (const auto effect : sg->mEffects) {
		if (effect->mIsAccept == isSgAccepted) {
			// if its effect of social move accepted
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

void UCiFManager::getAllSalientEffects(TArray<UCiFEffect*>& outEffects,
                                       UCiFSocialExchange* sg,
                                       const bool isAccepted,
                                       UCiFGameObject* initiator,
                                       UCiFGameObject* responder,
                                       const TArray<UCiFGameObject*> otherCast,
                                       TArray<UCiFGameObject*> levelCast)
{
	auto possibleOthers = otherCast;
	if (possibleOthers.IsEmpty()) {
		sg->getPossibleOthers(possibleOthers, initiator->mObjectName, responder->mObjectName);
	}
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

void UCiFManager::changeSocialState(UCiFSocialExchangeContext* sgContext, TArray<UCiFGameObject*> otherCast)
{
	const auto sg = mSocialExchangesLib->getSocialExchangeByName(sgContext->mGameName);
	const auto initiator = getGameObjectByName(sgContext->mInitiatorName);
	const auto responder = getGameObjectByName(sgContext->mResponderName);
	if (!sg) {
		UE_LOG(LogTemp, Error, TEXT("No social game '%s' found"), *(sgContext->mGameName.ToString()));
		return;
	}

	auto possibleOthers = otherCast;
	if (possibleOthers.IsEmpty()) {
		sg->getPossibleOthers(possibleOthers, initiator->mObjectName, responder->mObjectName);
	}

	const auto highestSaliencyEffect = sg->getEffectById(sgContext->mEffectId);
	checkf(highestSaliencyEffect != nullptr, TEXT("Effect wasn't found - this shouldn't happen at this stage"));
	const auto other = getGameObjectByName(sgContext->mOtherName);
	highestSaliencyEffect->mChange->valuation(initiator, responder, other);

	highestSaliencyEffect->mLastSeenTime = mTime;

	mSFDB->addContext(sgContext);

	//update all of the status to be one turn older now that we've chosen salient effects
	//in other words, the status lives "through" this spot in cif.time
	//and new statuses are not decremented yet, as they start on the next time step.
	// statuses that reached the end of their lifetime added as trigger context
	// to fire the necessary changes when finished.
	for (auto c : possibleOthers) {
		//for now, just update the possible others (i.e. people who aren't present don't change)
		for (auto &[statusType, statusArrWrapper] : c->mStatuses) {
			for (const UCiFGameObjectStatus* status : statusArrWrapper.statusArray) {
				if (status->mHasDuration && status->mRemainingDuration <= 1) {
					// creating predicate to remove the status
					auto pred = NewObject<UCiFPredicate>();
					pred->setStatusPredicate(c->mObjectName, status->mDirectedTowards, status->mType, status->mInitialDuration, false, true);

					// remove the status due to end of duration
					const auto directedToward = getGameObjectByName(status->mDirectedTowards);
					pred->valuation(c, directedToward);

					// make trigger context for this change in state
					const auto trigger = NewObject<UCiFTrigger>(mWorldContextObject);
					trigger->mId = UCiFTrigger::mStatusTimeoutTriggerID;
					const auto changeRule = NewObject<UCiFRule>(mWorldContextObject);
					changeRule->mPredicates.Add(pred);

					UCiFTriggerContext* triggerContext = trigger->makeTriggerContext(mTime, c, directedToward);
					triggerContext->mStatusTimeoutChange = changeRule;
					mSFDB->addContext(triggerContext);
				}
			}
		}

		// decrement status counters of all players
		c->updateStatusDurations(1);
	}

	//now that we have changed the state, updated statuses, we should run the triggers.
	//for now, triggers will happen *at the same cif time* as the change that caused them to trigger
	//
	//NOTE: I think the status.remainingTime should not be updated until after this call, to be consistent, but
	// that would require not updating the status.time's that were made the case from triggers and.... well, no. Not now.
	// the down side to this is that some statuses will have been made no more, that should probably be considered in the triggers
	mSFDB->runTriggers(possibleOthers);

	//increment system time after the context has been added
	mTime++;
}

TArray<UCiFRuleRecord*> UCiFManager::getPredicateRelevance(UCiFSocialExchange* sg,
                                                           UCiFGameObject* initiator,
                                                           UCiFGameObject* responder,
                                                           UCiFGameObject* other,
                                                           const FName forRole,
                                                           TArray<UCiFGameObject*> otherCast,
                                                           const FName mode)
{
	if (initiator->mGameObjectType != ECiFGameObjectType::CHARACTER || responder->mGameObjectType != ECiFGameObjectType::CHARACTER) {
		return {};
	}

	auto possibleOthers = otherCast;
	if (possibleOthers.IsEmpty()) {
		sg->getPossibleOthers(possibleOthers, initiator->mObjectName, responder->mObjectName);
	}
	const auto initAsChar = static_cast<UCiFCharacter*>(initiator);
	const auto resAsChar = static_cast<UCiFCharacter*>(responder);

	float totalNegScore = 0, totalPosScore = 0, totalScore = 0;
	TArray<UCiFRuleRecord*> relevantNegRR, relevantPosRR, relevantRR;

	// look through the rule records and pull out the important once. Also add MT definitions to the influence rules
	UCiFCharacter* role = nullptr;
	if (forRole == "initiator") {
		role = initAsChar;
	}
	else if (forRole == "responder") {
		role = resAsChar;
	}

	if (role) {
		for (const auto rr : role->mProspectiveMemory->mRuleRecords) {
			if ((rr->mInitiator == role->mObjectName) && (rr->mResponder == resAsChar->mObjectName)) {
				if (rr->mType == ERuleRecordType::SOCIAL_EXCHANGE) {
					if (rr->mName == sg->mName) {
						auto rrWeight = rr->mInfluenceRule->mWeight;
						if (rrWeight < 0) {
							totalNegScore += rrWeight;
							relevantNegRR.Add(rr);
						}
						else {
							totalPosScore += rrWeight;
							relevantPosRR.Add(rr);
						}
						totalScore += rrWeight;
						relevantRR.Add(rr);
					}
				}
				else if (rr->mType == ERuleRecordType::MICROTHEORY) {
					auto rrIntentIndex = rr->mInfluenceRule->findIntentIndex();
					if (rrIntentIndex < 0) {
						UE_LOG(LogTemp, Error, TEXT("Microtheory %s has a rule record without an intent"), *(rr->mName.ToString()));
					}
					else {
						auto rrIntentType = rr->mInfluenceRule->mPredicates[rrIntentIndex]->getIntentType();
						if (sg->mIntents[0]->mPredicates[0]->getIntentType() == rrIntentType) {
							auto mt = getMicrotheoryByName(rr->mName);
							auto newRR = NewObject<UCiFRuleRecord>();
							newRR->init(rr->mName, rr->mInitiator, rr->mResponder, rr->mOther, rr->mType, rr->mInfluenceRule);
							for (const auto p : mt->mDefinition->mPredicates) {
								newRR->mInfluenceRule->mPredicates.Add(p);
							}

							auto rrWeight = newRR->mInfluenceRule->mWeight;
							if (rrWeight < 0) {
								totalNegScore += rrWeight;
								relevantNegRR.Add(newRR);
							}
							else {
								totalPosScore += rrWeight;
								relevantPosRR.Add(newRR);
							}
							totalScore += rrWeight;
							relevantRR.Add(newRR);
						}
					}
				}
			}
		}
	}

	// at this point we have 2 vectors of the relevant pos and neg IRs
	//If we are interested in why the responder rejected...
	//if (mode == "reject" && forRole == "responder")
	//{
	//relevantRuleRecords = relevantNegRuleRecords;
	//totalScore = Math.abs(totalNegScore);
	//for each (ruleRecord in relevantRuleRecords)
	//{
	//ir = ruleRecord.influenceRule;
	//Debug.debug(this,"reject rule: " + ir.toString())
	//ir.weight = Math.abs(ir.weight);
	//}
	//}
	//else
	//{
	//otherwise, we are only interested in the positive reasons why someone did something
	//relevantRuleRecords = relevantPosRuleRecords;
	//totalScore = totalPosScore;
	//}

	// at this point relevantRR holds all the info we are interested in
	// now go over all the relevantRR and break them into their predicate pieces (each as its own rule record)
	TArray<UCiFRuleRecord*> uniquePredicateRuleRecords;
	for (const auto rr : relevantRR) {
		// before we can determine the number of predicates in relevant rulerecord we need to know how many
		// intent type preds to not include in the count
		int numIntents = 0;
		for (const auto p : rr->mInfluenceRule->mPredicates) {
			if (p->mIsIntent) {
				numIntents++;
			}
		}
		for (const auto p : rr->mInfluenceRule->mPredicates) {
			bool presentInUniquePredicateRuleRecords = false;
			if (!p->mIsIntent) {
				// do not count preds that are the second half of medium networks?????
				if (!((p->mComparatorType == EComparatorType::LESS_THAN) && (p->mNetworkValue == 67))) {
					// if this predicate has not been seen yet. to determine this, we need to go through all of the uniquePredicateRuleRecords
					for (const auto uniqueRR : uniquePredicateRuleRecords) {
						if (*p == *(uniqueRR->mInfluenceRule->mPredicates[0]) && (rr->mOther == uniqueRR->mOther)) {
							presentInUniquePredicateRuleRecords = true;
							uniqueRR->mInfluenceRule->mWeight += float(rr->mInfluenceRule->mWeight) / (rr->mInfluenceRule->mPredicates.Num()
								- numIntents);
						}
					}

					if (!presentInUniquePredicateRuleRecords) {
						const auto newRR = NewObject<UCiFRuleRecord>();
						const auto influenceRule = NewObject<UCiFInfluenceRule>();
						influenceRule->mPredicates.Add(p);
						influenceRule->mWeight = float(rr->mInfluenceRule->mWeight) / (rr->mInfluenceRule->mPredicates.Num() - numIntents);
						newRR->mInfluenceRule = influenceRule;
						newRR->init(rr->mName, rr->mInitiator, rr->mResponder, rr->mOther, rr->mType, influenceRule);
					}
				}
			}
		}
	}

	totalScore = 0; // TODO: why were we accumulating the total score while not doing anything with it
	TArray<UCiFRuleRecord*> outUniqueRRs;
	for (auto ruleRecord : uniquePredicateRuleRecords) {
		if (forRole == "responder" && (mode == "reject" || mode == "negative")) {
			if (ruleRecord->mInfluenceRule->mWeight < 0) {
				// if we are looking for responder reject rules, we want the highest negative
				ruleRecord->mInfluenceRule->mWeight = abs(ruleRecord->mInfluenceRule->mWeight);
				outUniqueRRs.Add(ruleRecord);
				totalScore += ruleRecord->mInfluenceRule->mWeight;
			}
		}
		else if (ruleRecord->mInfluenceRule->mWeight > 0) {
			outUniqueRRs.Add(ruleRecord);
			totalScore += ruleRecord->mInfluenceRule->mWeight;
		}
	}

	outUniqueRRs.Sort([](const UCiFRuleRecord& a, const UCiFRuleRecord& b) {
		return a.mInfluenceRule->mWeight <= b.mInfluenceRule->mWeight;
	});

	// now that we have the influence rules we need to normalize the weights
	for (auto ruleRecord : outUniqueRRs) {
		ruleRecord->mInfluenceRule->mWeight = round(ruleRecord->mInfluenceRule->mWeight / totalScore * 100);
	}

	return outUniqueRRs;
}

UCiFMicrotheory* UCiFManager::getMicrotheoryByName(const FName mtName)
{
	auto mt = mMicrotheoriesLib.Find(mtName);
	if (mt) {
		return *mt;
	}
	return nullptr;
}

void UCiFManager::getAllGameObjects(TArray<UCiFGameObject*>& outGameObjs)
{
	// TODO - optimizations, maybe after the first call for this, store all these game objects
	// in a member and return it, instead of every time running over all the objects
	for (const auto x : mCast->mCharacters) outGameObjs.Add(x);
	for (const auto x : mItemArray) outGameObjs.Add(x);
	for (const auto x : mKnowledgeArray) outGameObjs.Add(x);
}

void UCiFManager::getAllGameObjectsNames(TArray<FName>& outObjNames)
{
	for (auto x : mCast->mCharacters) outObjNames.Add(x->mObjectName);
	for (auto x : mItemArray) outObjNames.Add(x->mObjectName);
	for (auto x : mKnowledgeArray) outObjNames.Add(x->mObjectName);
}

TArray<FName> UCiFManager::getAllGameObjectsNames()
{
	TArray<FName> names;
	for (auto x : mCast->mCharacters) names.Add(x->mObjectName);
	for (auto x : mItemArray) names.Add(x->mObjectName);
	for (auto x : mKnowledgeArray) names.Add(x->mObjectName);

	return names;
}

void UCiFManager::getAllGameObjectsOfType(TArray<UCiFGameObject*>& outGameObjs, const ECiFGameObjectType type)
{
	switch (type) {
		case ECiFGameObjectType::CHARACTER:
			for (const auto c : mCast->mCharacters) outGameObjs.Add(c);
			break;
		case ECiFGameObjectType::ITEM:
			for (const auto i : mItemArray) outGameObjs.Add(i);
			break;
		case ECiFGameObjectType::KNOWLEDGE:
			for (const auto k : mKnowledgeArray) outGameObjs.Add(k);
			break;
	}
}

int8 UCiFManager::getNetworkWeightByType(const ESocialNetworkType netType, const uint8 id1, const uint8 id2) const
{
	auto net = mSocialNetworks.Find(netType);
	if (net) {
		return (*net)->getWeight(id1, id2);
	}

	UE_LOG(LogTemp, Error, TEXT("Couldn't find network of type %d"), netType);
	return 0;
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
