// Fill out your copyright notice in the Description page of Project Settings.


#include "Demo/DemoCifImplementation.h"
#include "CiFCharacter.h"
#include "CiFManager.h"
#include "CiFPredicate.h"
#include "CiFProspectiveMemory.h"
#include "CiFRule.h"
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
				outSocialGamesNames.Emplace(gs.mName, intentString);
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

