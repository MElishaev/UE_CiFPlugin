// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CiFGameScore.h"
#include "Utilities.h"
#include "UObject/Object.h"
#include "CiFProspectiveMemory.generated.h"

enum class ESocialNetworkType : uint8;
enum class ERelationshipType : uint8;
enum class EStatus : uint8;
class UCiFGameObject;
enum class EIntentType : uint8;
class UCiFCharacter;
class UCiFRuleRecord;

struct FCacheKey
{
	EIntentType mIntentType;

	union
	{
		EStatus mStatusType;
		ERelationshipType mRelationshipType;
		ESocialNetworkType mNetworkType;
	} IntentBasedEnum;

	bool operator==(const FCacheKey& Other) const;
	bool operator!=(const FCacheKey& Other) const { return !(*this == Other); }
	friend uint32 GetTypeHash(const FCacheKey& Key);
};

/**
 * Character specific prospective memory. This needs to be cleared each round.
 */
UCLASS()
class CIF_API UCiFProspectiveMemory : public UObject
{
	GENERATED_BODY()

public:
	void init();
	void initializeIntentScoreCache();

	void cacheIntentScore(const UCiFGameObject* responder, const FCacheKey extendedIntentType, const Score_t score);
	void addSocialExchangeScore(const FName seName, const FName initator, const FName responder, const FName other, const Score_t score);

	Score_t getIntentScore(const UCiFCharacter* responder, FCacheKey extendedIntentType);

	/**
	 * Returns the N highest scored games in prospective memory.
	 * 
	 * @param	count The number of the highest scored games to return.
	 */
	UFUNCTION(BlueprintCallable)
	TArray<FGameScore> getNHighestGameScores(uint8 count = 5);

	/**
	 * Searches the prospective memory for the highest game scores WRT another character.
	 * @param	responderName	The name of the other character.
	 * @param	count		The number of game scores to return.
	 * @param	minVolition The minimum scores to return. Maybe we don't care if all top scores are -100. it means the character
	 *						Don't want to do those stuff
	 * @return	The returned scores.
	 */
	TArray<FGameScore> getHighestGameScoresTo(const FName responderName, uint8 count = 5, const Score_t minVolition = -100);

	/**
	 * Fills output param game score with the score of the matching input params
	 * @param gameName The social exchange name
	 * @param responder The responder of the social exchange TODO-why specifically the responder and not also the initiator or other?
	 * @param outputScore Output param to be filled in
	 * @return True if found a game score matches the input params, false otherwise
	 */
	bool getGameScoreByName(const FName gameName, const UCiFCharacter* responder, FGameScore outputScore);

	Score_t getDefaultIntentScore() const { return DEFAULT_INTENT_SCORE; }

	UFUNCTION(BlueprintCallable)
	void printGameScores(UPARAM(ref) const TArray<FGameScore>& scores);

	/* Resets the object to its default state */
	void clear();

public:
	bool mIsCleared; // indicates if the prospective memory is clear before starting forming scores and storing here

	TArray<FGameScore> mScores;
	TArray<UCiFRuleRecord*> mRuleRecords; // array of all rules that evaluated to true while forming intent
	TArray<UCiFRuleRecord*> mResponseSeRuleRecords;

	/* A array of maps where each map represents the cache of calculated intents.
	 * each map holds scores for each combinations of keys of intentType*secondaryValue (for example, ADD_STATUS*STATUS_TYPE; INCREASE_NET*NETWORK_TYPE etc)
	 */
	TArray<TMap<FCacheKey, Score_t>> mIntentScoreCacheNew;

	Score_t DEFAULT_INTENT_SCORE = -100; // TODO - change to static member
};
