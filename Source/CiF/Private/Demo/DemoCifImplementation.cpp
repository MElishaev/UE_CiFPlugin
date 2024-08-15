// Fill out your copyright notice in the Description page of Project Settings.


#include "Demo/DemoCifImplementation.h"
#include "CiFCharacter.h"
#include "CiFManager.h"
#include "CiFPredicate.h"
#include "CiFProspectiveMemory.h"
#include "CiFRule.h"
#include "CiFRuleRecord.h"
#include "CiFSocialExchangeContext.h"
#include "CiFSocialExchangesLibrary.h"
#include "Demo/CifNPC.h"

void UDemoCifImplementation::prepareSocialGameOptionsWithCharacter(TArray<FSocialGameIntentPair>& outSocialGamesNames,
                                                                   ACifNPC* initiator,
                                                                   UCiFGameObject* responder,
                                                                   int32 numSocialGames,
                                                                   const bool isShowIntent,
                                                                   const bool isNPC)
{
	checkf(mCifManager != nullptr, TEXT("CiF manager wasn't initialized in the implementation"));
	auto init = static_cast<UCiFCharacter*>(mCifManager->getGameObjectByName(initiator->mCifCharacterComp->mObjectName));
	mCifManager->formIntentForSocialGames(init, responder);

	// taking into account the last N moves the initiator taken in case it would want to take one of the recently taken
	// actions, which we want to prevent him doing, so he won't spam the highest actions and do varied things.
	int32 numOfSGToSearchFor = numSocialGames + initiator->mLastSocialMoves.Num();
	auto gameScores = init->mProspectiveMemory->getHighestGameScoresTo(responder->mObjectName, numOfSGToSearchFor);

	// If quest is complete and we're talking to the quest completer, give the player the option of completing the quest
	// if (this.curQuest && this.curQuest.checkForCompletion(initiatorName, responderName) && 
	// 	initiatorName.toLowerCase() == "player" && responderName.toLowerCase() == curQuest.questCompleter.toLowerCase()) {
	// 	numSocialMoves--;
	// 	resultString += "<a href=\"event:Complete Quest\">Complete Quest</a>\n";
	// 	}

	// fill the output array up to the number of social games requested while
	// filtering out the social games that were taken in the last N moves by initiator
	if (!gameScores.IsEmpty()) {
		for (const auto gs : gameScores) {
			if (!initiator->isSocialGameInHistory(gs.mName)) {
				FString intentString = "";
				if (isShowIntent) {
					const auto socialGame = mCifManager->mSocialExchangesLib->getSocialExchangeByName(gs.mName);
					socialGame->mIntents[0]->mPredicates[0]->toIntentNLGString(intentString);
				}
				outSocialGamesNames.Emplace(gs.mName, intentString, gs.mScore);
				numSocialGames--;
				if (numSocialGames <= 0) {
					break;
				}
			}
		}
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("No social moves found"));
	}
}

bool UDemoCifImplementation::registerAsGameObject(const FName objectName, UCiFGameObject*& gameObjectCompRef)
{
	gameObjectCompRef = mCifManager->getGameObjectByName(objectName);
	if (!gameObjectCompRef) {
		UE_LOG(LogTemp, Error, TEXT("Couldn't find game object with name %s"), *objectName.ToString())
		return false;
	}
	return true;
}

void UDemoCifImplementation::offerOthers(TArray<UCiFGameObject*>& outOthers,
                                         const FName sgName,
                                         ACifNPC* initiator,
                                         const FName responder,
                                         bool isNPC)
{
	const auto sg = mCifManager->mSocialExchangesLib->getSocialExchangeByName(sgName);
	UCiFGameObject* responderComp = mCifManager->getGameObjectByName(responder);
	if (sg->mIsRequiresOther) {
		// Find all the possible others (TODO: WILL NEED TO BE UPDATED WITH SEEN/KNOW)
		TArray<UCiFGameObject*> possibleOthers;
		sg->getPossibleOthers(possibleOthers, initiator->mCifCharacterComp->mObjectName, responder);
		for (const auto object : possibleOthers) {
			// Characters/Items should have been seen by the initiator (player) and know exist
			// if (object->hasStatus(EStatus::KNOWN_BY, initiator)) {
			outOthers.Add(object);
			// }
		}
	}
	else {
		// call moveChosen if no others are required
		UE_LOG(LogTemp, Log, TEXT("No others are required for this interaction"));
		moveChosen(sgName, initiator, responderComp, isNPC);
	}
}

void UDemoCifImplementation::otherChosen(ACifNPC* initiator,
                                         UCiFGameObject* responder,
                                         UCiFGameObject* other,
                                         const FName sgName,
                                         const bool isNPC)
{
	// Passes the other the player selected to moveChosen
	// Does this move need to select an effect when handling people? (Giving Gift)
	// if (mCifManager->mSocialExchangesLib->getSocialExchangeByName(sgName)->needsSecondOther) {
	// offerEffects(sgName, initiator, responder, other);
	// }
	// else {
	moveChosen(sgName, initiator, responder, isNPC, other);
	// }
}

void UDemoCifImplementation::offerEffects(TArray<UCiFEffect*>& outEffects,
                                          const FName sgName,
                                          ACifNPC* initiator,
                                          UCiFGameObject* responder,
                                          UCiFGameObject* otherChosen)
{
	auto sg = mCifManager->mSocialExchangesLib->getSocialExchangeByName(sgName);

	TArray<UCiFGameObject*> allGameObjs;
	mCifManager->getAllGameObjects(allGameObjs);

	if (otherChosen) {
		// this is for NPC responders
		TArray<UCiFGameObject*> possibleOthers = {otherChosen};
		mCifManager->getAllSalientEffects(outEffects, sg, true, initiator->mCifCharacterComp, responder, possibleOthers, allGameObjs);
	}
	else {
		// this is for items (when player selected item and now he can USE or GIVE or whatever with this item)
		mCifManager->getAllSalientEffects(outEffects, sg, true, initiator->mCifCharacterComp, responder, allGameObjs, allGameObjs);
	}
}

void UDemoCifImplementation::itemMoveChosen(const FName sgName,
                                            ACifNPC* initiator,
                                            UCiFGameObject* responder,
                                            const bool isNPCPlaying,
                                            UCiFEffect* effect)
{
	FString resultString; // todo-this is in mismanor because their game is based on text, and they attach the events to the pressed text, which is not my case

	auto sg = mCifManager->mSocialExchangesLib->getSocialExchangeByName(sgName);
	TArray<UCiFGameObject*> characters;
	mCifManager->getAllGameObjectsOfType(characters, ECiFGameObjectType::CHARACTER);
	auto context = mCifManager->playGame(sg, initiator->mCifCharacterComp, responder, nullptr, {}, characters, effect);
	if (context->mResponderScore < 0) {
		if (isNPCPlaying) {
			resultString = responder->mObjectName.ToString() + " rejects " + sgName.ToString() + "\n";
		}
		else {
			resultString = responder->mObjectName.ToString() + " rejects " + sgName.ToString();
		}
	}
	else {
		if (isNPCPlaying) {
			resultString = responder->mObjectName.ToString() + " accepts " + sgName.ToString() + "\n";
		}
		else {
			resultString = responder->mObjectName.ToString() + " accepts " + sgName.ToString();
		}
	}

	handleItemMoveEffects(context);

	/* todo display dialogue related to move
	// figure out what instantiation will be shown to the player
	// displayed as lines of dialog
	curDialog = realizeDialog(sg, context, initiator, responder as GameObject);
	FString initiatorDisplayName = initiator->mCifCharacterComp->mObjectName.ToString();
	FString responderDisplayName = responder->mObjectName.ToString();

	if (initiator->mCifCharacterComp->mObjectName == "Player")
		initiatorDisplayName = playerName;
	else if (responder->mObjectName == "Player")
		responderDisplayName = playerName;

	for (auto lod : curDialog.lines)
	{
		if (lod.initiatorLine != "") {
			if (isNPCPlaying) {
				resultString += lod.initiatorLine + "\n";
			}
			else {
				resultString += lod.initiatorLine + "<br>";
			}
		}
		if (lod.responderLine != "") {
			if (isNPCPlaying) {
				resultString += lod.responderLine + "\n";
			}
			else {
				resultString += lod.responderLine + "<br>";
			}
		}
	}
	*/

	/* todo handle plot point related stuff
	movesSincePlotPoint++;
	// put in the output that a plot point has been found
	if (this.plotPointFound) {
		plotPointPool.updatePool();
		movesSincePlotPoint = 0;
		resultString += "PLOTPOINT: " + this.plotPointUsedName;
		if (isNPCPlaying)
			resultString += "\n";
		else
			resultString += "<br>";

		resultString += "ACTIVE-PLOTS: ";
		for (auto pp : PlotPoint in
		plotPointPool.activePlotPoints
		)
		{
			resultString += pp.name + " ";
		}
		resultString += "\n";

		this.plotPointFound = false;
		this.plotPointUsedName = "";
	}
	*/

	/* todo seems like a debug logs
	FString effectStr;
	sg->getEffectById(context->mEffectId)->toString(effectStr);
	systemMessage += effectStr;
	
	if (context->mResponderScore >= 0) {
		for (auto r :(mCifManager->getPredicateRelevance(sg,
		                                             initiator->mCifCharacterComp,
		                                             responder,
		                                             mCifManager->getGameObjectByName(context->mOtherName),
		                                             "responder",
		                                             {},
		                                             "positive"))) {
			//trace(r);
		}
	}
	else {
		for (auto r : (mCifManager->getPredicateRelevance(sg,
													 initiator->mCifCharacterComp,
													 responder,
													 mCifManager->getGameObjectByName(context->mOtherName),
													 "responder",
													 {},
													 "negative"))) {
			//trace(r);
		}
	}
	*/

	mCifManager->changeSocialState(context);

	// no longer waiting for player input to finish the social move todo - im leaving it here so i know that after this, the move is finished and game can continue
	// gameWaitingForInput = false;

	UE_LOG(LogTemp, Log, TEXT("item move: %s"), *resultString);
}

void UDemoCifImplementation::moveChosen(const FName sgName,
                                        ACifNPC* initiator,
                                        UCiFGameObject* responder,
                                        bool isNPC,
                                        UCiFGameObject* other,
                                        UCiFEffect* effect)
{
	const auto sg = mCifManager->mSocialExchangesLib->getSocialExchangeByName(sgName);

	// track what move the player chosen
	initiator->addMove(sgName);
	TArray<UCiFGameObject*> possibleOthers = {other};

	TArray<UCiFGameObject*> allGameObjects;
	mCifManager->getAllGameObjects(allGameObjects);

	UCiFSocialExchangeContext* sgContext = mCifManager->playGame(sg,
	                                                             initiator->mCifCharacterComp,
	                                                             responder,
	                                                             nullptr,
	                                                             possibleOthers,
	                                                             allGameObjects,
	                                                             effect);

	// todo-this is in mismanor because their game is based on text, and they attach the events to the pressed text, which is not my case
	FString resultString;
	if (sgContext->mResponderScore < 0) {
		resultString = responder->mObjectName.ToString() + " rejects " + sgName.ToString();
	}
	else {
		resultString = responder->mObjectName.ToString() + " accepts " + sgName.ToString();
	}

	// skipped resetting urges - no idea what it is for now

	// skipped realizing dialogue - not sure yet how the realization of SG will be present

	// skipped maintaining plot point related stuff for now

	FString temp;
	if (sgContext->mResponderScore >= 0) {
		// store the most influential rule record for this move
		auto ruleRecords = mCifManager->getPredicateRelevance(sg,
		                                                      initiator->mCifCharacterComp,
		                                                      responder,
		                                                      mCifManager->getGameObjectByName(sgContext->mOtherName),
		                                                      "responder",
		                                                      {},
		                                                      "positive");
		if (!ruleRecords.IsEmpty()) {
			ruleRecords[0]->toNLG(temp);
		}
	}
	else {
		auto ruleRecords = mCifManager->getPredicateRelevance(sg,
		                                                      initiator->mCifCharacterComp,
		                                                      responder,
		                                                      mCifManager->getGameObjectByName(sgContext->mOtherName),
		                                                      "responder",
		                                                      {},
		                                                      "negative");
		if (!ruleRecords.IsEmpty()) {
			ruleRecords[0]->toNLG(temp);
		}
	}

	if (sgContext->mResponderScore >= 0) {
		if (!temp.IsEmpty()) {
			resultString += sg->mName.ToString() + " accepted by " + responder->mObjectName.ToString() + " because " + temp + "\n";
		}
		else {
			resultString += sg->mName.ToString() + " accepted by " + responder->mObjectName.ToString() + " by default\n";
		}
	}
	else {
		if (!temp.IsEmpty()) {
			resultString += sg->mName.ToString() + " rejected by " + responder->mObjectName.ToString() + " because " + temp + "\n";
		}
		else {
			resultString += sg->mName.ToString() + " rejected by " + responder->mObjectName.ToString() + " by default\n";
		}
	}

	mCifManager->changeSocialState(sgContext);
	handleItemMoveEffects(sgContext);

	// no longer waiting for player input to finish social move
	if (isNPC && sgContext) {
		auto sgEffect = sg->getEffectById(sgContext->mEffectId);
		FString effectStr;
		sgEffect->toString(effectStr);
		resultString += "\n" + effectStr + "\n";
		resultString += "Social state\n";

		for (auto p : sgEffect->mChange->mPredicates) {
			if (p->mType == EPredicateType::NETWORK) {
				UCiFGameObject *first=nullptr, *second=nullptr;

				auto primaryRole = p->getRoleValue(p->mPrimary);
				if (primaryRole == "initiator") first = initiator->mCifCharacterComp;
				else if (primaryRole == "responder") first = responder;
				else if (primaryRole == "other") first = mCifManager->getGameObjectByName(sgContext->mOtherName);

				auto secondaryRole = p->getRoleValue(p->mSecondary);
				if (secondaryRole == "initiator") second = initiator->mCifCharacterComp;
				else if (secondaryRole == "responder") second = responder;
				else if (secondaryRole == "other") second = mCifManager->getGameObjectByName(sgContext->mOtherName);

				auto networkEnum = StaticEnum<ESocialNetworkType>();
				resultString += networkEnum->GetValueAsString(p->mNetworkType) + ": ";
				resultString += first->mObjectName.ToString() + "-->" + second->mObjectName.ToString() + ": ";
				resultString += mCifManager->getNetworkWeightByType(p->mNetworkType, first->mNetworkId, second->mNetworkId) + "\n";
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("move chosen: %s"), *resultString);
}

void UDemoCifImplementation::effectChosen(const FName sgName,
                                          ACifNPC* initiator,
                                          UCiFGameObject* responder,
                                          const bool isNPC,
                                          UCiFEffect* effect,
                                          UCiFGameObject* other)
{
	auto sg = mCifManager->mSocialExchangesLib->getSocialExchangeByName(sgName);

	// Passes the other and effect the player selected to moveChosen()
	if (sg->mResponderType == ECiFGameObjectType::CHARACTER) {
		moveChosen(sgName, initiator, responder, isNPC, other, effect);
	}
	else if (sg->mResponderType == ECiFGameObjectType::ITEM) {
		itemMoveChosen(sgName, initiator, responder, isNPC, effect);
	}
	// TODO: what this method should return?
}

void UDemoCifImplementation::handleItemMoveEffects(UCiFSocialExchangeContext* context)
{
	auto changeRule = context->getChange();
	checkf(changeRule != nullptr, TEXT("Change rule is nullptr"));

	for (const auto p : changeRule->mPredicates) {
		// TODO: This stuff should all be taken care of in valuation, not done by hand here 

		// NOTE: The order of these two checks matters in the document!:
		// In giving an item, the first player must drop it, then the second will pick it up.  

		// If the post-effects of the move indicate the item should not be held, then drop it
		// Note: This assumes that in the effect preconditions that the "Held By" status was TRUE
		// and that the item was in the initiator's game inventory before the move
		// ALSO note that different cases for NPCs may be required
		if (p->mType == EPredicateType::STATUS) {
			if (p->mStatusType == EStatus::HELD_BY || p->mStatusType == EStatus::IS_HOLDING) {
				// if the responder is an item
				auto initiator = mCifManager->getGameObjectByName(context->mInitiatorName);
				auto responder = mCifManager->getGameObjectByName(context->mResponderName);
				auto other = mCifManager->getGameObjectByName(context->mOtherName);
				if (p->mPrimary == "responder") {
					if ((p->mSecondary == "initiator") && (initiator->mGameObjectType == ECiFGameObjectType::CHARACTER) &&
						(responder->mGameObjectType == ECiFGameObjectType::ITEM)) {
						if (p->mIsNegated) {
							// TODO - drop item to world
						}
						else {
							// TODO -- pick item from world
						}
					}
				}
				else if (p->mPrimary == "other" && other) {
					auto item = mCifManager->getGameObjectByName(other->mObjectName);
					if (p->mSecondary == "initiator" && (initiator->mGameObjectType == ECiFGameObjectType::CHARACTER) &&
						(other->mGameObjectType == ECiFGameObjectType::ITEM)) {
						if (p->mIsNegated) {
							// TODO - initiator drops the item
						}
						else {
							// TODO - initiator picks the item
						}
					}
					else if ((p->mSecondary == "responder") && (initiator->mGameObjectType == ECiFGameObjectType::CHARACTER) &&
						(other->mGameObjectType == ECiFGameObjectType::ITEM)) {
						if (p->mIsNegated) {
							// TODO - responder drops the item
						}
						else {
							// todo responder picks the item
						}
					}
				}
			}
		}
	}
}
