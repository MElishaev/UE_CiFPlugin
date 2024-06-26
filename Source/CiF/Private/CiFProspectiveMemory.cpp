// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFProspectiveMemory.h"

#include "CiFCast.h"
#include "CiFManager.h"
#include "CiFSubsystem.h"
#include "CiFPredicate.h"
#include "Kismet/GameplayStatics.h"

UCiFProspectiveMemory::UCiFProspectiveMemory()
{
	initializeIntentScoreCache();
}

void UCiFProspectiveMemory::initializeIntentScoreCache()
{
	auto cifManager = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UCiFSubsystem>()->getInstance();

	const auto numCharacters = cifManager->mCast->mCharacters.Num();

	mIntentScoreCache.SetNum(numCharacters);
	mIntentNegScoreCache.SetNum(numCharacters);
	mIntentPosScoreCache.SetNum(numCharacters);

	for (int i = 0; i < numCharacters; i++) {
		mIntentScoreCache[i].SetNum(static_cast<uint8>(EPredicateType::SIZE));
		mIntentNegScoreCache[i].SetNum(static_cast<uint8>(EPredicateType::SIZE));
		mIntentPosScoreCache[i].SetNum(static_cast<uint8>(EPredicateType::SIZE));

		for (int j = 0; j < static_cast<uint8>(EPredicateType::SIZE); j++) {
			mIntentScoreCache[i][j] = DEFAULT_INTENT_SCORE;
			mIntentNegScoreCache[i][j] = DEFAULT_INTENT_SCORE;
			mIntentPosScoreCache[i][j] = DEFAULT_INTENT_SCORE;
		}
	}
}

void UCiFProspectiveMemory::cacheIntentScore(const UCiFCharacter* responder, const EIntentType intentType, const int8 score)
{
	mIntentScoreCache[responder->mNetworkId][static_cast<uint8>(intentType)] = score;
}

void UCiFProspectiveMemory::addSocialExchangeScore(const FName seName,
                                                   const FName initator,
                                                   const FName responder,
                                                   const FName other,
                                                   const int8 score)
{
	mScores.Emplace(seName, initator, responder, other, score);
}

int8 UCiFProspectiveMemory::getIntentScore(const UCiFCharacter* responder, EIntentType intentType)
{
	return mIntentScoreCache[responder->mNetworkId][static_cast<uint8>(intentType)];
}

TArray<FGameScore> UCiFProspectiveMemory::getNHighestGameScores(uint8 count)
{
	mScores.Sort();
	TArray<FGameScore> topNScores;
	count = mScores.Num() < count ? mScores.Num() : count;

	for (int i = 0; i < count; i++) {
		topNScores.Add(mScores[i]);
	}

	return topNScores;
}

TArray<FGameScore> UCiFProspectiveMemory::getHighestGameScoresTo(const FName responderName, uint8 count, const int8 minVolition)
{
	TArray<FGameScore> allMatchingScoresAboveMinVolition;
	uint8 amountAdded = 0;
	for (const auto& score : mScores) {
		if (score.mResponder == responderName && score.mScore > minVolition) {
			allMatchingScoresAboveMinVolition.Add(score);
			amountAdded++;
		}
	}
	allMatchingScoresAboveMinVolition.Sort();

	// if we have less the requested passing scores, return the array
	if (allMatchingScoresAboveMinVolition.Num() <= count) {
		return allMatchingScoresAboveMinVolition;
	}

	// if we have more than the requested scores, filter the requested amount of scores
	TArray<FGameScore> highestNScores;
	for (size_t i = 0; i < count; i++) {
		highestNScores.Add(allMatchingScoresAboveMinVolition[i]);
	}

	return highestNScores;
}

bool UCiFProspectiveMemory::getGameScoreByName(const FName gameName, const UCiFCharacter* responder, FGameScore outputScore)
{
	auto gs = mScores.FindByPredicate([=](const FGameScore g) {
		return gameName == g.mName && responder->mObjectName == g.mResponder;
	});

	if (gs) {
		outputScore = *gs;
		return true;
	}

	return false;
}

void UCiFProspectiveMemory::clear()
{
	auto cifManager = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UCiFSubsystem>()->getInstance();

	const auto numCharacters = cifManager->mCast->mCharacters.Num();

	for (int i = 0; i < mIntentScoreCache.Num(); i++) {
		for (int j = 0; j < mIntentScoreCache[0].Num(); j++) {
			mIntentScoreCache[i][j] = DEFAULT_INTENT_SCORE;
			mIntentNegScoreCache[i][j] = DEFAULT_INTENT_SCORE;
			mIntentPosScoreCache[i][j] = DEFAULT_INTENT_SCORE;
		}
	}

	// TODO- reset the rest of the members - but need to make sure that this makes sense for the purpose of this function
	//			and this class. because maybe i want to still hold the container of the same size, like in the intent
	//			caches above.
	mResponseSeRuleRecords.Reset();
	mRuleRecords.Reset();
	mScores.Reset();
}
