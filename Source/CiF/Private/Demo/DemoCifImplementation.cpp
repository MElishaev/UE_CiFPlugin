// Fill out your copyright notice in the Description page of Project Settings.


#include "Demo/DemoCifImplementation.h"

#include "CiFCast.h"
#include "CiFCharacter.h"
#include "CiFManager.h"
#include "CiFPredicate.h"
#include "CiFProspectiveMemory.h"
#include "CiFRule.h"
#include "CiFRuleRecord.h"
#include "CiFSocialExchangeContext.h"
#include "CiFSocialExchangesLibrary.h"
#include "Demo/CifNPC.h"
#include "Narrative/CifNarrativeManager.h"
#include "CiFSocialExchange.h"
#include <GLSMacroses.h>

#include "Narrative/CifPlotPoint.h"

void UDemoCifImplementation::init()
{
    mCifNarrativeManager = NewObject<UCifNarrativeManager>(this);
    mCifNarrativeManager->init();
}

UCiFCharacter* UDemoCifImplementation::chooseNPCInitiatorForSocialGame()
{
	checkf(mCifManager != nullptr, TEXT("CiF manager wasn't initialized in the implementation"));
	const auto numOfChars = mCifManager->mCast->mCharacters.Num();
	mCharacterIndexInCast = (mCharacterIndexInCast + 1) % numOfChars;
	auto initiator = mCifManager->mCast->mCharacters[mCharacterIndexInCast];
	if (initiator->mObjectName == "Player") {
		mCharacterIndexInCast = (mCharacterIndexInCast + 1) % numOfChars;
		initiator = mCifManager->mCast->mCharacters[mCharacterIndexInCast];
	}
	GLS_LOG(LogTemp, Log, TEXT("Chosen initiator: %s"), *(initiator->mObjectName.ToString()));
	return initiator;
}

FGameScore UDemoCifImplementation::selectSocialGameFromList(const TArray<FGameScore>& sgs) const
{
	/* use scores as distribution function to select a social game */

	int32 totalScore = 0;
	for (const auto& sg : sgs) {
		totalScore += sg.mScore;
	}

	const auto selectedIndex = FMath::RandRange(0, totalScore - 1);
	int32 currentPos = 0;
	for (int i = 0; i < sgs.Num(); i++) {
		currentPos += sgs[i].mScore;
		if (selectedIndex < currentPos) {
			GLS_LOG(LogTemp, Log, TEXT("Selected social game: %s (r: %s, o: %s)"),
				*(sgs[i].mName.ToString()),
				*(sgs[i].mResponder.ToString()),
				*(sgs[i].mOther.ToString()));
			return sgs[i];
		}
	}
	checkf(false, TEXT("Shouldn't get here"));
	return {};
}

void UDemoCifImplementation::prepareSocialGameOptionsWithCharacter(TArray<FSocialGameIntentPair>& outSocialGamesNames,
                                                                   ACifNPC* initiator,
                                                                   UCiFGameObject* responder,
                                                                   int32 numSocialGames,
                                                                   const bool isShowIntent,
                                                                   const bool isNPC)
{
	checkf(mCifManager != nullptr, TEXT("CiF manager wasn't initialized in the implementation"));
	auto init = static_cast<UCiFCharacter*>(mCifManager->getGameObjectByName(initiator->mCifCharacterComp->mObjectName));
	mCifManager->clearProspectiveMemory();
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
		GLS_LOG(LogTemp, Warning, TEXT("No social moves found"));
	}
}

bool UDemoCifImplementation::registerAsGameObject(const FName objectName, UCiFGameObject*& gameObjectCompRef)
{
	gameObjectCompRef = mCifManager->getGameObjectByName(objectName);
	if (!gameObjectCompRef) {
		GLS_LOG(LogTemp, Error, TEXT("Couldn't find game object with name %s"), *objectName.ToString())
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
	if (sg->isOtherRequired()) {
		// Find all the possible others (TODO: WILL NEED TO BE UPDATED WITH SEEN/KNOW)
		sg->getPossibleOthers(outOthers, initiator->mCifCharacterComp->mObjectName, responder);
	}
	else {
		GLS_LOG(LogTemp, Log, TEXT("No others are required for this interaction"));
	}
}

UCiFSFDBContext* UDemoCifImplementation::otherChosen(ACifNPC* initiator,
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
	return handleChosenMove(sgName, initiator, responder, isNPC, other);
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

UCiFSFDBContext* UDemoCifImplementation::itemMoveChosen(const FName sgName,
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

	GLS_LOG(LogTemp, Log, TEXT("item move: %s"), *resultString);
    return context;
}

UCiFSFDBContext* UDemoCifImplementation::handleChosenMove(const FName sgName,
                                                          ACifNPC* initiator,
                                                          UCiFGameObject* responder,
                                                          bool isNPC,
                                                          UCiFGameObject* other,
                                                          UCiFEffect* effect)
{
	checkf(initiator, TEXT("Initiator must be != nullptr"));
	GLS_LOG(LogTemp, Log, TEXT("move chosen: %s (%s, %s, %s)"), *(sgName.ToString()), *(initiator->mCifCharacterComp->mObjectName.ToString()),
		*(responder->mObjectName.ToString()), other ? *(other->mObjectName.ToString()) : *FString("None"));
	const auto sg = mCifManager->mSocialExchangesLib->getSocialExchangeByName(sgName);

	// track what move the player chosen
	initiator->addMove(sgName);

    // prepare others array and all cast array
	TArray<UCiFGameObject*> possibleOthers = {};
	if (other) {
		possibleOthers.Add(other);
	}
	TArray<UCiFGameObject*> allGameObjects;
	mCifManager->getAllGameObjects(allGameObjects);
    
	// todo - if other is received in this method, why not send it to playGame? (it is nullptr instead)
	UCiFSocialExchangeContext* sgContext = mCifManager->playGame(sg,
	                                                             initiator->mCifCharacterComp,
	                                                             responder,
	                                                             nullptr,
	                                                             possibleOthers,
	                                                             allGameObjects,
	                                                             effect);
	
	// TODO: skipped resetting urges - no idea what it is for now
    //  but by the name, i can assume that this can be something like ticking variable that increases over time
    //  or decreases based on if this social game fulfilled the urge, and this will influence the weighting of the
    //  next chosen SG for this initiator
    
	// TODO: skipped maintaining plot point related stuff for now

	mCifManager->changeSocialState(sgContext);
	handleItemMoveEffects(sgContext);

    // for debugging or could be for in-game log of what happened and why
	FString resultStr;
	generateResultString(sgContext, isNPC, resultStr);
	GLS_LOG(LogTemp, Log, TEXT("move result: %s"), *resultStr);

    return sgContext;
}

UCiFSFDBContext* UDemoCifImplementation::effectChosen(const FName sgName,
                                                      ACifNPC* initiator,
                                                      UCiFGameObject* responder,
                                                      const bool isNPC,
                                                      UCiFEffect* effect,
                                                      UCiFGameObject* other)
{
	auto sg = mCifManager->mSocialExchangesLib->getSocialExchangeByName(sgName);

	// Passes the other and effect the player selected to moveChosen()
	if (sg->mResponderType == ECiFGameObjectType::CHARACTER) {
		return handleChosenMove(sgName, initiator, responder, isNPC, other, effect);
	}
	else if (sg->mResponderType == ECiFGameObjectType::ITEM) {
		return itemMoveChosen(sgName, initiator, responder, isNPC, effect);
	}
    checkf(false, TEXT("Shouldn't get here. Responder should be a character or an item (couldn't be knowledge)"));
    return nullptr;
}

void UDemoCifImplementation::handleItemMoveEffects(const UCiFSocialExchangeContext* context)
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

UCifInstantiation* UDemoCifImplementation::debug_preparePlotPointDialogue(FName revealerName) const
{
    FCifPlotPointSelection ppSelection;
    if (!mCifNarrativeManager->findPlotPointToPlay(revealerName, ppSelection)) {
        return nullptr;
    }
    return mCifNarrativeManager->getInstantiationForPlotPoint(ppSelection);
}

void UDemoCifImplementation::generateResultString(const UCiFSocialExchangeContext* sgContext, const bool isNPC, FString& outStr) const
{
	const auto initiator = mCifManager->getGameObjectByName(sgContext->mInitiatorName);
	const auto responder = mCifManager->getGameObjectByName(sgContext->mResponderName);
	const auto sg = mCifManager->getSocialGameByName(sgContext->mGameName);

	// store the most influential rule record for this move
	FString rr;
	if (sgContext->mResponderScore >= 0) {
		auto ruleRecords = mCifManager->getPredicateRelevance(sg,
															  initiator,
															  responder,
															  mCifManager->getGameObjectByName(sgContext->mOtherName),
															  "responder",
															  {},
															  "positive");
		if (!ruleRecords.IsEmpty()) {
			ruleRecords[0]->toNLG(rr);
		}
	}
	else {
		auto ruleRecords = mCifManager->getPredicateRelevance(sg,
															  initiator,
															  responder,
															  mCifManager->getGameObjectByName(sgContext->mOtherName),
															  "responder",
															  {},
															  "negative");
		if (!ruleRecords.IsEmpty()) {
			ruleRecords[0]->toNLG(rr);
		}
	}

	const auto sgEffect = sg->getEffectById(sgContext->mEffectId);
	if (sgEffect->mIsAccept) {
		if (!rr.IsEmpty()) {
			outStr += sg->mName.ToString() + " accepted by " + responder->mObjectName.ToString() + " because " + rr + "\n";
		}
		else {
			outStr += sg->mName.ToString() + " accepted by " + responder->mObjectName.ToString() + " by default\n";
		}
	}
	else {
		if (!rr.IsEmpty()) {
			outStr += sg->mName.ToString() + " rejected by " + responder->mObjectName.ToString() + " because " + rr + "\n";
		}
		else {
			outStr += sg->mName.ToString() + " rejected by " + responder->mObjectName.ToString() + " by default\n";
		}
	}

	if (isNPC && sgContext) {
		FString effectStr;
		sgEffect->toString(effectStr);
		outStr += "\nEffect:" + effectStr + "\n";
		outStr += "Updated social state:\n";

		for (auto p : sgEffect->mChange->mPredicates) {
			if (p->mType == EPredicateType::NETWORK) {
				UCiFGameObject *first=nullptr, *second=nullptr;

				auto primaryRole = p->getRoleValue(p->mPrimary);
				if (primaryRole == "initiator") first = initiator;
				else if (primaryRole == "responder") first = responder;
				else if (primaryRole == "other") first = mCifManager->getGameObjectByName(sgContext->mOtherName);

				auto secondaryRole = p->getRoleValue(p->mSecondary);
				if (secondaryRole == "initiator") second = initiator;
				else if (secondaryRole == "responder") second = responder;
				else if (secondaryRole == "other") second = mCifManager->getGameObjectByName(sgContext->mOtherName);

				outStr += enumToStringNoPrefix(p->mNetworkType) + ": ";
				outStr += first->mObjectName.ToString() + "-->" + second->mObjectName.ToString() + ": ";
				outStr += FString::FromInt(mCifManager->getNetworkWeightByType(p->mNetworkType, first->mNetworkId, second->mNetworkId)) + "\n";
			}
			else if (p->mType == EPredicateType::STATUS) {
				UCiFGameObject *first=nullptr, *second=nullptr;

				auto primaryRole = p->getRoleValue(p->mPrimary);
				if (primaryRole == "initiator") first = initiator;
				else if (primaryRole == "responder") first = responder;
				else if (primaryRole == "other") first = mCifManager->getGameObjectByName(sgContext->mOtherName);

				auto secondaryRole = p->getRoleValue(p->mSecondary);
				if (secondaryRole == "initiator") second = initiator;
				else if (secondaryRole == "responder") second = responder;
				else if (secondaryRole == "other") second = mCifManager->getGameObjectByName(sgContext->mOtherName);

				outStr += enumToStringNoPrefix(p->mStatusType) + ": ";
				outStr += first->mObjectName.ToString();
				if (second) {
					outStr += "-->" + second->mObjectName.ToString();
				}
				outStr += ", " + FString::FromInt(p->mStatusDuration) + "\n";
			}
			else if (p->mType == EPredicateType::RELATIONSHIP) {
				UCiFGameObject *first=nullptr, *second=nullptr;

				auto primaryRole = p->getRoleValue(p->mPrimary);
				if (primaryRole == "initiator") first = initiator;
				else if (primaryRole == "responder") first = responder;
				else if (primaryRole == "other") first = mCifManager->getGameObjectByName(sgContext->mOtherName);

				auto secondaryRole = p->getRoleValue(p->mSecondary);
				if (secondaryRole == "initiator") second = initiator;
				else if (secondaryRole == "responder") second = responder;
				else if (secondaryRole == "other") second = mCifManager->getGameObjectByName(sgContext->mOtherName);

				outStr += first->mObjectName.ToString();
				outStr += " " + enumToStringNoPrefix(p->mRelationshipType) + " with ";
				outStr += second->mObjectName.ToString();
			}
		}
	}
}
