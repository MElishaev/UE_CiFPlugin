﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CiFGameScore.h"
#include "UObject/Object.h"
#include "CiFProspectiveMemory.generated.h"

class UCiFGameObject;
enum class EIntentType : uint8;
class UCiFCharacter;
class UCiFRuleRecord;
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

	void cacheIntentScore(const UCiFGameObject* responder, const EIntentType intentType, const int8 score);
	void addSocialExchangeScore(const FName seName, const FName initator, const FName responder, const FName other, const int8 score);

	int8 getIntentScore(const UCiFCharacter* responder, EIntentType intentType);

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
	TArray<FGameScore> getHighestGameScoresTo(const FName responderName, uint8 count = 5, const int8 minVolition = -100);
	
	/**
	 * Fills output param game score with the score of the matching input params
	 * @param gameName The social exchange name
	 * @param responder The responder of the social exchange TODO-why specifically the responder and not also the initiator or other?
	 * @param outputScore Output param to be filled in
	 * @return True if found a game score matches the input params, false otherwise
	 */
	bool getGameScoreByName(const FName gameName, const UCiFCharacter* responder, FGameScore outputScore);

	int8 getDefaultIntentScore() const { return DEFAULT_INTENT_SCORE; }

	UFUNCTION(BlueprintCallable)
	void printGameScores(UPARAM(ref) const TArray<FGameScore>& scores);
	
	/* Resets the object to its default state */
	void clear();
public:

	bool mIsCleared; // indicates if the prospective memory is clear before starting forming scores and storing here
	
	TArray<FGameScore> mScores;
	TArray<UCiFRuleRecord*> mRuleRecords; // array of all rules that evaluated to true while forming intent
	TArray<UCiFRuleRecord*> mResponseSeRuleRecords;

	/* A two dimensional array where intentScoreCache[x][y] where x is a character id and y refers to the intent id */
	TArray<TArray<int8>> mIntentScoreCache;
	TArray<TArray<int8>> mIntentPosScoreCache;
	TArray<TArray<int8>> mIntentNegScoreCache;
	
	int8 DEFAULT_INTENT_SCORE = -100; // TODO - change to static member
};
