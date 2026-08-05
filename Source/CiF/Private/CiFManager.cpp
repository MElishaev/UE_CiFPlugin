// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFManager.h"

#include <GLSMacroses.h>

#include "Misc/Paths.h"
#include "CiFCast.h"
#include "CiFCharacter.h"
#include "CiFCulturalKnowledgeBase.h"
#include "CiFInfluenceRule.h"
#include "Narrative/CifInstantiation.h"
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
#include "Subsystems/MKUI_LoadingScreenSubsystem.h"

UCiFManager::UCiFManager()
{
	mTime = 0;
}

void UCiFManager::init(const UObject* worldContextObject)
{
	mWorldContextObject = const_cast<UObject*>(worldContextObject);

	mSocialExchangesLib = NewObject<UCiFSocialExchangesLibrary>(mWorldContextObject);
	mSFDB = NewObject<UCiFSocialFactsDataBase>(mWorldContextObject);
	// TODO - its not correct to put it here. it should happen on init but on "start game" or something, because if the
	// player has already has save game, we need to just load it from the save game, although it should be the same data.
	// for now i'll put it here

	const FString sgLibPath = FPaths::Combine(*FPaths::ProjectPluginsDir(), *FString("CiF/Content/Data/socialGameLib.json"));
	GLS_LOG(LogTemp, Log, TEXT("Reading social games from %s"), *sgLibPath);
	loadSocialGameLib(sgLibPath, worldContextObject);

	const FString mtLibPath = FPaths::Combine(*FPaths::ProjectPluginsDir(), *FString("CiF/Content/Data/microtheories.json"));
	GLS_LOG(LogTemp, Log, TEXT("Reading microtheories from %s"), *mtLibPath);
	loadMicrotheories(mtLibPath, worldContextObject);

	const FString castPath = FPaths::Combine(*FPaths::ProjectPluginsDir(), *FString("CiF/Content/Data/cast.json"));
	GLS_LOG(LogTemp, Log, TEXT("Reading cast from %s"), *castPath);
	loadCast(castPath, worldContextObject);

	const FString itemsPath = FPaths::Combine(*FPaths::ProjectPluginsDir(), *FString("CiF/Content/Data/items.json"));
	GLS_LOG(LogTemp, Log, TEXT("Reading items from %s"), *itemsPath);
	loadItemList(itemsPath, worldContextObject);

	const FString knowledgePath = FPaths::Combine(*FPaths::ProjectPluginsDir(), *FString("CiF/Content/Data/knowledgeList.json"));
	GLS_LOG(LogTemp, Log, TEXT("Reading knowledge list from %s"), *knowledgePath);
	loadKnowledgeList(knowledgePath, worldContextObject);

	const FString sfdbPath = FPaths::Combine(*FPaths::ProjectPluginsDir(), *FString("CiF/Content/Data/sfdb.json"));
	GLS_LOG(LogTemp, Log, TEXT("Reading SFDB from %s"), *sfdbPath);
	loadSFDB(sfdbPath, worldContextObject);

	const FString triggersPath = FPaths::Combine(*FPaths::ProjectPluginsDir(), *FString("CiF/Content/Data/triggers.json"));
	GLS_LOG(LogTemp, Log, TEXT("Reading triggers from %s"), *triggersPath);
	loadTriggers(triggersPath, worldContextObject);

	const FString socialNetworksPath = FPaths::Combine(*FPaths::ProjectPluginsDir(), *FString("CiF/Content/Data/socialNetworks.json"));
	GLS_LOG(LogTemp, Log, TEXT("Reading social networks from %s"), *socialNetworksPath);
	loadSocialNetworks(socialNetworksPath, worldContextObject);

	const FString ckbPath = FPaths::Combine(*FPaths::ProjectPluginsDir(), *FString("CiF/Content/Data/ckb.json"));
	GLS_LOG(LogTemp, Log, TEXT("Reading CKB from %s"), *ckbPath);
	loadCKB(ckbPath, worldContextObject);

	GLS_LOG(LogTemp, Log, TEXT("Finished loading all"));
    mbInitialized = true;

    auto loadingScreenSubsystem = worldContextObject->GetWorld()->GetGameInstance()->GetSubsystem<UMKUI_LoadingScreenSubsystem>();
    if (loadingScreenSubsystem) {
        loadingScreenSubsystem->notifyStageComplete("cif");
    }
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

	const auto microtheoriesJson = jsonObject->GetArrayField(TEXT("Microtheories"));
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

	const auto itemsJson = jsonObject->GetArrayField(TEXT("Items"));
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

	const auto knowledgeJson = jsonObject->GetArrayField(TEXT("Knowledge"));
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

	const auto scsJson = jsonObject->GetArrayField(TEXT("StatusContext"));
	for (const auto scJson : scsJson) {
		auto sc = UCiFStatusContext::loadFromJson(scJson->AsObject(), worldContextObject);
		if (sc) {
			mSFDB->mContexts.Add(sc);
		}
	}

	const auto tcsJson = jsonObject->GetArrayField(TEXT("TriggerContext"));
	for (const auto tcJson : tcsJson) {
		auto tc = UCiFTriggerContext::loadFromJson(tcJson->AsObject(), worldContextObject);
		if (tc) {
			mSFDB->mContexts.Add(tc);
		}
	}

	const auto sgcsJson = jsonObject->GetArrayField(TEXT("SocialGameContext"));
	for (const auto sgJson : sgcsJson) {
		auto sgc = UCiFSocialExchangeContext::loadFromJson(sgJson->AsObject(), worldContextObject);
		if (sgc) {
			mSFDB->mContexts.Add(sgc);
		}
		else {
			GLS_LOG(LogTemp, Warning, TEXT("SocialGmaeContext failed to load from file"));
		}
	}

	// Dont load backstory context for now, they include characters not available in game.
	// const auto backstoryJson = jsonObject->GetArrayField(TEXT("BackstoryContext"));
	// for (const auto bsJson : backstoryJson) {
	// 	auto bsc = UCiFSocialExchangeContext::loadFromJson(bsJson->AsObject(), worldContextObject);
	// 	if (bsc) {
	// 		mSFDB->mContexts.Add(bsc);
	// 	}
	// 	else {
	// 		GLS_LOG(LogTemp, Warning, TEXT("SocialGmaeContext failed to load from file"));
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

	const auto snsJson = jsonObject->GetArrayField(TEXT("SocialNetworks"));
	for (const auto snJson : snsJson) {
		auto sn = UCiFSocialNetwork::loadFromJson(snJson->AsObject(), worldContextObject);
		mSocialNetworks.Add(sn->mType, sn);
	}

	const auto rsJson = jsonObject->GetObjectField(TEXT("RelationshipNetwork"));
	mRelationshipNetworks = UCiFRelationshipNetwork::loadFromJson(rsJson, worldContextObject);
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

	const auto triggersJson = jsonObject->GetArrayField(TEXT("Triggers"));
	for (const auto triggerJson : triggersJson) {
		auto t = UCiFTrigger::loadFromJson(triggerJson->AsObject(), worldContextObject);
		if (t) {
			mSFDB->mTriggers.Add(t);
		}
		else {
			GLS_LOG(LogTemp, Warning, TEXT("Trigger failed to load from file"));
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
	clearProspectiveMemory();

	for (const auto responder : mCast->mCharacters) {
		if (responder->mObjectName != initiator->mObjectName) {
			formIntentForSocialGames(initiator, responder, static_cast<TArray<UCiFGameObject*>>(mCast->mCharacters));
		}
	}
}

bool UCiFManager::formIntent(UCiFCharacter* initiator, const TSet<FName>& responders)
{
    clearProspectiveMemory();
    
    // this is called after the clear memory because we might try to use the formed intent but if we fail because of this
    // we don't want to use the old memory
    if (responders.IsEmpty()) {
        GLS_LOG(LogTemp, Warning, TEXT("Shouldn't call this method with no responders - something went wrong"));
        return false;
    }
    
    for (const auto responderName : responders) {
        if (const auto responder = mCast->getCharByName(responderName)) {
            formIntentForSocialGames(initiator, responder, static_cast<TArray<UCiFGameObject*>>(mCast->mCharacters));
        }
        else {
            GLS_LOG(LogTemp, Error, TEXT("Could not find character '%s' in game's cast"), *responderName.ToString());
            return false;
        }
    }
    return true;
}

void UCiFManager::formIntentForSocialGames(UCiFCharacter* initiator,
                                           UCiFGameObject* responder,
                                           const TArray<UCiFGameObject*>& possibleOthers)
{
	GLS_LOG(LogTemp, Verbose, TEXT("Forming intent for %s->%s"),
		*(initiator->mObjectName.ToString()), *(responder->mObjectName.ToString()));
	for (auto [name, se] : mSocialExchangesLib->mSocialExchanges) {
		GLS_LOG(LogTemp, Verbose, TEXT("Forming intent for %s"), *(name.ToString()));
		formIntentForSpecificSocialExchange(se, initiator, responder, possibleOthers);
	}
}

void UCiFManager::formIntentForSpecificSocialExchange(UCiFSocialExchange* socialExchange,
                                                      UCiFCharacter* initiator,
                                                      UCiFGameObject* responder,
                                                      const TArray<UCiFGameObject*>& possibleOthers)
{
	if (socialExchange->isOtherRequired()) {
		if (possibleOthers.Num() == 0) {
			TArray<UCiFGameObject*> calculatedPossibleOthers;
			socialExchange->getPossibleOthers(calculatedPossibleOthers, initiator->mObjectName, responder->mObjectName);
		    if (!calculatedPossibleOthers.IsEmpty()) {
		        formIntentThirdParty(socialExchange, initiator, responder, calculatedPossibleOthers);
		    }
		}
		else {
			formIntentThirdParty(socialExchange, initiator, responder, possibleOthers);
		}
	}
	else {
		formIntentThirdParty(socialExchange, initiator, responder);
	}
}

void UCiFManager::formIntentThirdParty(UCiFSocialExchange* socialExchange,
                                       UCiFCharacter* initiator,
                                       UCiFGameObject* responder,
                                       const TArray<UCiFGameObject*>& possibleOthers)
{
	FScore_t score;
	UCiFGameObject* bestOther = nullptr; // in case the SE requires other, this will hold the other that resulted in the highest score

	if (socialExchange->checkPreconditionsVariableOther(initiator, responder, possibleOthers)) {
		/* we get here if preconditions hold. if other is needed, it means that there is at least
		 * 1 other that holds the precondition, and if other isn't needed then the preconditions hold
		 * for the initiator and responder. */
		
		// score the SG and if requires other, fills in the other that results in the best score
		score = socialExchange->scoreSocialExchange(initiator, responder, bestOther, possibleOthers);
		if (bestOther) {
			GLS_LOG(LogTemp, Verbose, TEXT("Social game requires other and other was chosen: %s"), *(bestOther->mObjectName.ToString()));
		}

		// checks if already cached MTs for the current SG intent (some social exchanges has the same intent, e.g. flirt / give romantic gift)
		// if not, score and cache
		const auto extendedIntentType = socialExchange->getSocialExchangeExtendedIntentType();
		if (not initiator->mProspectiveMemory->mIntentScoreCache[responder->mNetworkId].Contains(extendedIntentType)) {
			const auto mtsScore = scoreAllMicrotheoriesForType(socialExchange, initiator, responder, possibleOthers);
			initiator->mProspectiveMemory->cacheIntentScore(responder, extendedIntentType, mtsScore);
			score += mtsScore;
			GLS_LOG(LogTemp, VeryVerbose, TEXT("Microtheories contributed %d"), mtsScore.val);
		}
		else {
			score += *(initiator->mProspectiveMemory->mIntentScoreCache[responder->mNetworkId].Find(extendedIntentType));
			GLS_LOG(LogTemp, VeryVerbose, TEXT("Cached microtheories contribution %d"),
				(*(initiator->mProspectiveMemory->mIntentScoreCache[responder->mNetworkId].Find(extendedIntentType))).val);
		}
	}
	else {
		// if SE doesn't pass the preconditions with the specified characters combination
		score = initiator->mProspectiveMemory->getDefaultIntentScore();
	}

	initiator->mProspectiveMemory->addSocialExchangeScore(socialExchange->mName,
	                                                      initiator->mObjectName,
	                                                      responder->mObjectName,
	                                                      bestOther ? bestOther->mObjectName : NAME_None,
	                                                      score);

	
	// todo - debug print all rule that contributed score
	const auto key = FRRMapKey(socialExchange->mName, initiator->mObjectName,
		responder->mObjectName, bestOther ? bestOther->mObjectName : NAME_None);
	auto rrWrapper = initiator->mProspectiveMemory->mRuleRecordsMap.Find(key);
	if (rrWrapper) {
		FString rulesStr = "Contributing rules (" + FString::FromInt(rrWrapper->mRuleRecords.Num()) + ") for " + socialExchange->mName.ToString() +
			"(" + rrWrapper->mRuleRecords[0]->mInitiator.ToString() + "," + rrWrapper->mRuleRecords[0]->mResponder.ToString() + "," +
				rrWrapper->mRuleRecords[0]->mOther.ToString() + ") score " + FString::FromInt(score) + "\n";
		for (const auto rule : rrWrapper->mRuleRecords) {
			FString localRuleStr;
			rule->toDebugNLG(localRuleStr);
			rulesStr += localRuleStr + "\n";
		}
		GLS_LOG(LogTemp, Verbose, TEXT("%s"), *rulesStr);
	}
}

FScore_t UCiFManager::scoreAllMicrotheoriesForType(UCiFSocialExchange* se,
                                                   UCiFCharacter* initiator,
                                                   UCiFGameObject* responder,
                                                   const TArray<UCiFGameObject*>& possibleOthers)
{
	TArray<UCiFGameObject*> others = possibleOthers.Num() > 0 ? possibleOthers : static_cast<TArray<UCiFGameObject*>>(mCast->mCharacters);
	FScore_t totalScore = 0;

	for (const auto [name, microTheory] : mMicrotheoriesLib) {
		totalScore += microTheory->score(initiator, responder, se, others);
	}

	return totalScore;
}

UCiFSocialExchangeContext* UCiFManager::playGame(UCiFSocialExchange* sg,
                                                 UCiFGameObject* initiator,
                                                 UCiFGameObject* responder,
                                                 UCiFGameObject* other,
                                                 const TArray<UCiFGameObject*>& otherCast,
                                                 const TArray<UCiFGameObject*>& levelCast,
                                                 UCiFEffect* chosenEffect)
{
	//we figure out who the other is in THIS function, when we call 'getSalientOtherAndEffect'
	//Since other part of this function depend on other being null, we are going
	//to just set it to null here explicitly (since passing in, say, an instantiated yet 'blank' character with no name
	//will cause issues).
	other = nullptr;

	if (levelCast.IsEmpty()) {
		GLS_LOG(LogTemp, Warning, TEXT("Level cast is empty, this is not allowed - but why?"));
	}

	TArray<UCiFGameObject*> possibleOthers = otherCast;
	if (possibleOthers.IsEmpty()) {
		sg->getPossibleOthers(possibleOthers, initiator->mObjectName, responder->mObjectName);
	}

	const auto responderScore = getResponderScore(sg, initiator, responder, possibleOthers);
	GLS_LOG(LogTemp, Warning, TEXT("Responder score %d"), responderScore.val);

	const bool isAcceptGameIntent = (responderScore >= 0);

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
		GLS_LOG(LogTemp, Error, TEXT("This shouldn't happen. Didn't find effect for a social game, meaning it is meaningless SG"));
		return nullptr;
	}
    GLS_LOG(LogTemp,
            Log,
            TEXT("Effect with ID %d and other %s were chosen"),
            mostSalientEffect->mId,
            mostSalientOther ? *mostSalientOther->mObjectName.ToString() : *FString());

	// the other to use when all cases of other being passed in a third character being needed when one is not provided
	UCiFGameObject* trueOther = (!other && sg->isOtherRequired()) ? mostSalientOther : other;

	/* Preparing social game context for output */
	const auto socialGameContext = createSgContext(sg, initiator, responder, trueOther, mostSalientEffect, responderScore);

	return socialGameContext;
}

FScore_t UCiFManager::getResponderScore(UCiFSocialExchange* sg,
                                        UCiFGameObject* initiator,
                                        UCiFGameObject* responder,
                                        const TArray<UCiFGameObject*>& activeOtherCast)
{
	TArray<UCiFGameObject*> possibleOthers = activeOtherCast;
	if (possibleOthers.IsEmpty()) {
		sg->getPossibleOthers(possibleOthers, initiator->mObjectName, responder->mObjectName);
	}

	UCiFGameObject* discard;
	FScore_t score = sg->scoreSocialExchange(static_cast<UCiFCharacter*>(initiator), responder, discard, possibleOthers, true);

	// score MT - look up responder's intent to play social game with initiator
	if (responder->mGameObjectType == ECiFGameObjectType::CHARACTER) {
		const auto r = static_cast<UCiFCharacter*>(responder);
		const auto extendedIntentIndex = sg->mIntents[0]->mPredicates[0]->getExtendedIntentType();
		// todo - is there data here? because we don't call formIntentAll, i don't think the responder has
		//  any intent stored for this game. it may be that he need to run all MTs...
		if (r->mProspectiveMemory->mIntentScoreCache[initiator->mNetworkId].Contains(extendedIntentIndex)) {
			score += *(r->mProspectiveMemory->mIntentScoreCache[initiator->mNetworkId].Find(extendedIntentIndex));
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
		// todo - why not searching salient rejection effects?
		if (effect->mIsAccept == isSgAccepted) {
			// if its effect of social move accepted
			if (sg->isOtherRequired()) {
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
							if (effect->mCondition->evaluate(initiator, responder, o, sg) && (sg->mOtherType == o->mGameObjectType)) {
								possibleSalientEffects.Add(effect);
								possibleSalientOthers.Add(o);
							}
						}
					}
				}
			}
			else {
				if (effect->mCondition->evaluate(initiator, responder, nullptr, sg)) {
					possibleSalientEffects.Add(effect);
					possibleSalientOthers.Add(nullptr);
				}
			}
		}
	}

	/* after storing all the possible effects and their others (if any)
	 * choose the most appropriate effect to be played */
	int32 maxSaliency = -9999;
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
			if (sg->isOtherRequired()) {
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
		GLS_LOG(LogTemp, Error, TEXT("No social game '%s' found"), *(sgContext->mGameName.ToString()));
		return;
	}

	auto possibleOthers = otherCast;
	if (possibleOthers.IsEmpty()) {
		getAllGameObjectsOfType(possibleOthers, ECiFGameObjectType::CHARACTER);
	}

	const auto highestSaliencyEffect = sg->getEffectById(sgContext->mEffectId);
	checkf(highestSaliencyEffect != nullptr, TEXT("Effect wasn't found - this shouldn't happen at this stage"));
	const auto other = getGameObjectByName(sgContext->mOtherName);

	// update status duration before applying current SG changes because they made add
	// statuses which we don't want to reduce their duration
	for (auto o : possibleOthers) {
		o->updateStatusDurations(1);
	}

	// apply the social change
	highestSaliencyEffect->mChange->valuation(initiator, responder, other);
	highestSaliencyEffect->mLastSeenTime = mTime;

	mSFDB->addContext(sgContext);

	//update all of the status to be one turn older now that we've chosen salient effects
	//in other words, the status lives "through" this spot in cif.time
	//and new statuses are not decremented yet, as they start on the next time step.
	// statuses that reached the end of their lifetime added as trigger context, after
	// it was valuated by a newly created predicate negating the status, ordering its removal
	for (auto c : possibleOthers) {
		//for now, just update the possible others (i.e. people who aren't present don't change)
		for (auto mapIt = c->mStatuses.CreateIterator(); mapIt; ++mapIt) {
			// todo this pair could be deleted while inside loop
			// using iterator because it safely allows to delete elements (statuses) while iterating over the array
			for (auto it = mapIt->Value.statusArray.CreateIterator(); it; ++it) {
				// todo array element could be deleted while inside loop
				if ((*it)->mHasDuration && (*it)->mRemainingDuration < 1) {
					// creating predicate to remove the status
					auto pred = NewObject<UCiFPredicate>(mWorldContextObject);
					pred->setStatusPredicate(c->mObjectName, (*it)->mDirectedTowards, (*it)->mType, (*it)->mInitialDuration, false, true);

					// remove the status due to end of duration
					const auto directedToward = getGameObjectByName((*it)->mDirectedTowards);
					pred->valuation(c, directedToward);

					// make trigger context for this change in state
					const auto trigger = NewObject<UCiFTrigger>(mWorldContextObject);
					trigger->mId = UCiFTrigger::mStatusTimeoutTriggerID;
					const auto changeRule = NewObject<UCiFRule>(mWorldContextObject);
					changeRule->mPredicates.Add(pred);
					trigger->mChange = changeRule;

					UCiFTriggerContext* triggerContext = trigger->makeTriggerContext(mTime, c, directedToward);
					triggerContext->mStatusTimeoutChange = changeRule;
					mSFDB->addContext(triggerContext);
				}
			}
		}
	}

	//now that we have changed the state, updated statuses, we should run the triggers.
	//for now, triggers will happen *at the same cif time* as the change that caused them to trigger
	//
	//NOTE: I think the status.remainingTime should not be updated until after this call, to be consistent, but
	// that would require not updating the status.time's that were made the case from triggers and.... well, no. Not now.
	// the down side to this is that some statuses will have been made no more, that should probably be considered in the triggers
	mSFDB->runTriggers(possibleOthers);

	// todo - assuming i add a delegate here, i have a delegate for each possible change, how should i extract the exact change?
	//  for example, if the change is of social network, how can i update only the cell that was changed and not
	//  go through all the matrices again to update the UI, this feels very un-optimized.
	//  i could start with partial optimization of only notifying for a specific type of social state change
	notifySocialStateChange(highestSaliencyEffect);

	//increment system time after the context has been added
	mTime++;
}

TArray<UCiFRuleRecord*> UCiFManager::getPredicateRelevance(const UCiFSocialExchange* sg,
                                                           const UCiFGameObject* initiator,
                                                           const UCiFGameObject* responder,
                                                           const UCiFGameObject* other,
                                                           const FName forRole,
                                                           const TArray<UCiFGameObject*>& otherCast,
                                                           const FName mode) const
{
	if (initiator->mGameObjectType != ECiFGameObjectType::CHARACTER || responder->mGameObjectType != ECiFGameObjectType::CHARACTER) {
		GLS_LOG(LogTemp, Error, TEXT("Doesn't make sense that non the initiator neither the responder are characters in the SG"));
		return {};
	}

	if (forRole == "initiator" && (mode == "reject" || mode == "negative")) {
		GLS_LOG(LogTemp, Warning, TEXT("We are not interested/should not reach here for a social exchange that the initiator rejected"));
		return {};
	}

	auto possibleOthers = otherCast;
	if (possibleOthers.IsEmpty()) {
		sg->getPossibleOthers(possibleOthers, initiator->mObjectName, responder->mObjectName);
	}

	const UCiFCharacter* role = nullptr;
	if (forRole == "initiator") {
		role = static_cast<const UCiFCharacter*>(initiator);
	}
	else if (forRole == "responder") {
		role = static_cast<const UCiFCharacter*>(responder);
	}

	float totalNegScore = 0, totalPosScore = 0;
	TArray<UCiFRuleRecord*> relevantNegRR, relevantPosRR;

	// look through the rule records and pull out the important ones. Also add MT definitions to the influence rules
	if (role) {
		const auto key = FRRMapKey(sg->mName, initiator->mObjectName, responder->mObjectName, other ? other->mObjectName : NAME_None);
		const auto ruleRecordsWrapper = role->mProspectiveMemory->mRuleRecordsMap.Find(key);
		if (!ruleRecordsWrapper) {
			GLS_LOG(LogTemp, Warning, TEXT("No rule records were found when looking for relevant RRs of %s for this SG"),
				*(role->mObjectName.ToString()));
			return {};
		}
		for (const auto rr : ruleRecordsWrapper->mRuleRecords) {
			if ((rr->mInitiator == initiator->mObjectName) && (rr->mResponder == responder->mObjectName)) {
				if (rr->mType == ERuleRecordType::SOCIAL_EXCHANGE) {
					if (rr->mName == sg->mName) {
						const auto rrWeight = rr->mInfluenceRule->mWeight;
						if (rrWeight < 0) {
							totalNegScore += rrWeight;
							relevantNegRR.Add(rr);
						}
						else {
							totalPosScore += rrWeight;
							relevantPosRR.Add(rr);
						}
					}
				}
				else if (rr->mType == ERuleRecordType::MICROTHEORY) {
					// if the MT has the same intent of the SG, create a new rule record
					// that holds the 
					const auto rrIntentIndex = rr->mInfluenceRule->findIntentIndex();
					if (rrIntentIndex < 0) {
						GLS_LOG(LogTemp, Error, TEXT("Microtheory %s has a rule record without an intent"), *(rr->mName.ToString()));
					}
					else {
						const auto rrIntentType = rr->mInfluenceRule->mPredicates[rrIntentIndex]->getExtendedIntentType();
						if (sg->mIntents[0]->mPredicates[0]->getExtendedIntentType() == rrIntentType) {
							auto mt = getMicrotheoryByName(rr->mName);
							auto newRR = NewObject<UCiFRuleRecord>();
							newRR->init(rr->mName, rr->mInitiator, rr->mResponder, rr->mOther, rr->mType, rr->mInfluenceRule);
							for (const auto p : mt->mDefinition->mPredicates) {
								newRR->mInfluenceRule->mPredicates.Add(p);
							}

							const auto rrWeight = newRR->mInfluenceRule->mWeight;
							if (rrWeight < 0) {
								totalNegScore += rrWeight;
								relevantNegRR.Add(newRR);
							}
							else {
								totalPosScore += rrWeight;
								relevantPosRR.Add(newRR);
							}
						}
					}
				}
			}
		}
	}

	/*
	 * at this point we have 2 vectors of the relevant pos and neg IRs.
	 * now we will choose which one of them to use based on request by the input parameters
	 */

	const float totalScore = ((mode == "reject" || mode == "negative") && forRole == "responder") ? FMath::Abs(totalNegScore) : totalPosScore;
	const auto& relevantRR = ((mode == "reject" || mode == "negative") && forRole == "responder") ? relevantNegRR : relevantPosRR;
	if ((mode == "reject" || mode == "negative") && forRole == "responder") {
		for (const auto ruleRecord : relevantRR) {
			auto ir = ruleRecord->mInfluenceRule;
			ir->mWeight = FMath::Abs(ir->mWeight);
		}
	}

	// todo runtime complexity wise - this doesn't look good when scaled up with numbers

	/*
	 * at this point relevantRR holds all the info we are interested in.
	 * now go over all the relevantRR and break them into their predicate pieces
	 * (each predicate as its own rule record, because some influence rule consist
	 * of several predicates including intent predicates - you can see this
	 * especially in the Microtheories influence rules)
	 */
	TArray<UCiFRuleRecord*> uniquePredicateRRs;
	for (const auto rr : relevantRR) {
		// before we can determine the number of predicates in relevant rule records we need to know how many
		// intent type preds to not include in the count - because intent type predicates doesn't really
		// indicate of a social state influencing the exchange but of the intent of the exchange
		int numIntents = 0;
		for (const auto p : rr->mInfluenceRule->mPredicates) {
			if (p->mIsIntent) {
				numIntents++;
			}
		}

		for (const auto p : rr->mInfluenceRule->mPredicates) {
			if (!p->mIsIntent) {
				bool isContainedInUniquePredicateRRs = false;
				// if this predicate has not been seen yet. to determine this, we need to go through all of the uniquePredicateRuleRecords
				for (const auto uniqueRR : uniquePredicateRRs) {
					// note on condition - uniqueRR are rules that are generated in the current for-loop and contain only 1 predicate
					if (*p == *(uniqueRR->mInfluenceRule->mPredicates[0]) && (rr->mOther == uniqueRR->mOther)) {
						isContainedInUniquePredicateRRs = true;
						uniqueRR->mInfluenceRule->mWeight += (rr->mInfluenceRule->mWeight) /
															 (rr->mInfluenceRule->mPredicates.Num() - numIntents);
					}
				}

				if (!isContainedInUniquePredicateRRs) {
					const auto newRR = NewObject<UCiFRuleRecord>();
					const auto influenceRule = NewObject<UCiFInfluenceRule>(mWorldContextObject);
					influenceRule->mPredicates.Add(p);
					influenceRule->mWeight = (rr->mInfluenceRule->mWeight) / (rr->mInfluenceRule->mPredicates.Num() - numIntents);
					newRR->init(rr->mName, rr->mInitiator, rr->mResponder, rr->mOther, rr->mType, influenceRule);
					uniquePredicateRRs.Add(newRR);
				}
			}
		}
	}

	uniquePredicateRRs.Sort([](const UCiFRuleRecord& a, const UCiFRuleRecord& b) {
		return a.mInfluenceRule->mWeight > b.mInfluenceRule->mWeight; // '>' because we want descending order
	});

	// now that we have the influence rules we need to normalize the weights
	for (const auto rr : uniquePredicateRRs) {
		rr->mInfluenceRule->mWeight = rr->mInfluenceRule->mWeight / totalScore;
	}

	return uniquePredicateRRs;
}

UCiFMicrotheory* UCiFManager::getMicrotheoryByName(const FName mtName) const
{
	auto mt = mMicrotheoriesLib.Find(mtName);
	if (mt) {
		return *mt;
	}
	return nullptr;
}

UCiFSocialExchange* UCiFManager::getSocialGameByName(const FName name) const
{
	auto sg = mSocialExchangesLib->getSocialExchangeByName(name);
	return sg;
}

void UCiFManager::getAllGameObjects(TArray<UCiFGameObject*>& outGameObjs) const
{
	// TODO - optimizations, maybe after the first call for this, store all these game objects
	// in a member and return it, instead of every time running over all the objects
	for (const auto x : mCast->mCharacters) outGameObjs.Add(x);
	for (const auto x : mItemArray) outGameObjs.Add(x);
	for (const auto x : mKnowledgeArray) outGameObjs.Add(x);
}

void UCiFManager::getAllGameObjectsNames(TArray<FName>& outObjNames) const
{
	for (auto x : mCast->mCharacters) outObjNames.Add(x->mObjectName);
	for (auto x : mItemArray) outObjNames.Add(x->mObjectName);
	for (auto x : mKnowledgeArray) outObjNames.Add(x->mObjectName);
}

void UCiFManager::getAllGameObjectsOfType(TArray<UCiFGameObject*>& outGameObjs, const ECiFGameObjectType type) const
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

	GLS_LOG(LogTemp, Error, TEXT("Couldn't find network of type %d"), netType);
	return 0;
}

void UCiFManager::notifySocialStateChange(const UCiFEffect* effect)
{
	for (const auto p : effect->mChange->mPredicates) {
		switch (p->mType) {
			case EPredicateType::NETWORK:
				OnSocialNetworkUpdated.Broadcast(p->mNetworkType);
				break;
			case EPredicateType::RELATIONSHIP:
				break;
		}
	}

	// TODO - always notify status changes because they are more dynamic and can change without intentional play from the player
}

UCiFSocialExchangeContext* UCiFManager::createSgContext(const UCiFSocialExchange* sg,
                                                        const UCiFGameObject* initiator,
                                                        const UCiFGameObject* responder,
                                                        const UCiFGameObject* other,
                                                        const UCiFEffect* chosenEffect,
                                                        const FScore_t& score) const
{
	const auto sgContext = NewObject<UCiFSocialExchangeContext>(mWorldContextObject);

	// TODO: for now this never happens because there is no CKB in any of the effects of a social game...
	//  so, I'm even not sure yet how this is used
	if (chosenEffect->hasCKBReference()) {
		sgContext->mChosenItemCKB = pickAGoodCKBObject(initiator, responder, chosenEffect->getCKBReferencePredicate());
	}

	sgContext->mGameName = sg->mName;
	sgContext->mEffectId = chosenEffect->mId;
	sgContext->mInitiatorName = initiator->mObjectName;
	sgContext->mResponderName = responder->mObjectName;

    if (const auto instantiationOfEffectId = sg->getInstantiationById(chosenEffect->mInstantiationId)) {
        sgContext->mPerformanceRealization = instantiationOfEffectId->getmDescription(initiator, responder, other);
    }

	if (chosenEffect->hasSFDBLabel()) {
		for (const auto p : chosenEffect->mChange->mPredicates) {
			if (p->mType == EPredicateType::SFDB_LABEL) {
				FSFDBLabel label;
				label.to = p->getSecondaryCharacterNameFromVariables(initiator, responder, other);
				label.from = p->getPrimaryCharacterNameFromVariables(initiator, responder, other);
				label.type = p->mSFDBLabel.type;
				sgContext->mSFDBLabels.Add(label);
			}
		}
	}

	sgContext->mOtherName = other ? other->mObjectName : NAME_None;
	sgContext->mTime = mTime;
	if (initiator->mGameObjectType != ECiFGameObjectType::CHARACTER) {
		sgContext->mInitiatorScore = 0;
	}
	sgContext->mResponderScore = score;

	return sgContext;
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

UCiFGameObject* UCiFManager::getGameObjectByNetworkId(const uint8 id) const
{
    return mCast->getCharByNetworkId(id);
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
    if (type == ESocialNetworkType::RELATIONSHIP) {
        return mRelationshipNetworks;
    }
    
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
