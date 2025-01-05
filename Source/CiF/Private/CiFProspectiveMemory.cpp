// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFProspectiveMemory.h"

#include "CiFCast.h"
#include "CiFManager.h"
#include "CiFSubsystem.h"
#include "CiFPredicate.h"
#include "Kismet/GameplayStatics.h"

void UCiFProspectiveMemory::init()
{
	initializeIntentScoreCache();
}

void UCiFProspectiveMemory::initializeIntentScoreCache()
{
	auto cifManager = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UCiFSubsystem>()->getInstance();

	const auto numCharacters = cifManager->mCast->mCharacters.Num();

	if (numCharacters <= 0) {
		UE_LOG(LogTemp, Warning, TEXT("Number of characters is %d"), numCharacters);
		return;
	}
	
	mIntentScoreCacheNew.SetNum(numCharacters);
}

void UCiFProspectiveMemory::cacheIntentScore(const UCiFGameObject* responder, const FCacheKey extendedIntentType, const Score_t score)
{
	mIntentScoreCacheNew[responder->mNetworkId].Add(extendedIntentType, score);
	mIsCleared = false; // todo - should it be here? in what cases we cache and does this needs to be reset before forming intents?
}

void UCiFProspectiveMemory::addSocialExchangeScore(const FName seName,
                                                   const FName initator,
                                                   const FName responder,
                                                   const FName other,
                                                   const int8 score)
{
	mScores.Emplace(seName, initator, responder, other, score);
	mIsCleared = false;
}

Score_t UCiFProspectiveMemory::getIntentScore(const UCiFCharacter* responder, FCacheKey extendedIntentType)
{
	auto scorePtr = mIntentScoreCacheNew[responder->mNetworkId].Find(extendedIntentType); 
	if (scorePtr) {
		return *scorePtr;
	}
	return DEFAULT_INTENT_SCORE;
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

void UCiFProspectiveMemory::printGameScores(const TArray<FGameScore>& scores)
{
	for (const auto& gs : scores) {
		UE_LOG(LogTemp, Log, TEXT("%s: %s -> %s (%s) == %d"),
			*(gs.mName.ToString()), *(gs.mInitiator.ToString()), *(gs.mResponder.ToString()),
			*(gs.mOther.ToString()), gs.mScore);
	}
}

void UCiFProspectiveMemory::clear()
{
	if (mIsCleared) {
		return;
	}
	
	auto cifManager = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UCiFSubsystem>()->getInstance();

	const auto numCharacters = cifManager->mCast->mCharacters.Num();

	for (int i = 0; i < mIntentScoreCacheNew.Num(); i++) {
		mIntentScoreCacheNew[i].Reset();
	}

	// TODO- reset the rest of the members - but need to make sure that this makes sense for the purpose of this function
	//			and this class. because maybe i want to still hold the container of the same size, like in the intent
	//			caches above.
	mResponseSeRuleRecords.Reset();
	mRuleRecords.Reset();
	mScores.Reset();

	mIsCleared = true;
}

bool FCacheKey::operator==(const FCacheKey& Other) const
{
	if (mIntentType != Other.mIntentType) return false;
	switch (mIntentType) {
		case EIntentType::INCREASE_NET:
		case EIntentType::DECREASE_NET:
			return IntentBasedEnum.mNetworkType == Other.IntentBasedEnum.mNetworkType;
		case EIntentType::ADD_STATUS:
		case EIntentType::REMOVE_STATUS:
			return IntentBasedEnum.mStatusType == Other.IntentBasedEnum.mStatusType;
		case EIntentType::START_RELATIONSHIP:
		case EIntentType::END_RELATIONSHIP:
			return IntentBasedEnum.mRelationshipType == Other.IntentBasedEnum.mRelationshipType;
	}
	return false;
}

uint32 GetTypeHash(const FCacheKey& Key)
{
	switch (Key.mIntentType) {
		case EIntentType::INCREASE_NET:
		case EIntentType::DECREASE_NET:
			return HashCombine(GetTypeHash(Key.mIntentType), GetTypeHash(Key.IntentBasedEnum.mNetworkType));
		case EIntentType::ADD_STATUS:
		case EIntentType::REMOVE_STATUS:
			return HashCombine(GetTypeHash(Key.mIntentType), GetTypeHash(Key.IntentBasedEnum.mStatusType));
		case EIntentType::START_RELATIONSHIP:
		case EIntentType::END_RELATIONSHIP:
			return HashCombine(GetTypeHash(Key.mIntentType), GetTypeHash(Key.IntentBasedEnum.mRelationshipType));
	}
	return 0;
}
