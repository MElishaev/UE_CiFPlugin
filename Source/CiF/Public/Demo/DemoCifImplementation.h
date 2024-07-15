// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CifImplementationBase.h"
#include "DemoCifImplementation.generated.h"

class UCiFGameObject;
class ACifNPC;
/**
 * Pairs the social game name and its intent string if the social game is of intent type
 */
USTRUCT(BlueprintType)
struct FSocialGameIntentPair
{
	GENERATED_BODY()
	FName socialGameName;
	FString intentString;
};

/**
 * 
 */
UCLASS(BlueprintType)
class CIF_API UDemoCifImplementation : public UCifImplementationBase
{
	GENERATED_BODY()

public:
	// TODO - maybe change this name later
	/**
	 * Prepares the available social games options when interacting with a character for a social game.
	 * @param outSocialGamesNames	Output array that holds in a pair the social game FName and its Intent string in case we want
	 *								to show the user how it will influence the social state
	 * @param initiator				The initiator of the social game
	 * @param responder				The responder to the social game
	 * @param numSocialGames		The max number of social games to show as options
	 * @param isShowIntent			True if we want to show how the social game influences the social state (e.g. "increases romance network")
	 * @param isNPC					True if the initiator is an NPC and not the player (so for this case of course we doesn't open UI or show anything)
	 */
	UFUNCTION(BlueprintCallable)
	void prepareSocialGameOptionsWithCharacter(TArray<FSocialGameIntentPair>& outSocialGamesNames,
											   ACifNPC* initiator,
											   UCiFGameObject* responder,
											   int32 numSocialGames,
											   const bool isShowIntent,
											   const bool isNPC = false);
};
