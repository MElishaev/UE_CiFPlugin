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

	UPROPERTY(BlueprintReadWrite)
	FName socialGameName;

	UPROPERTY(BlueprintReadWrite)
	FString intentString;

	UPROPERTY(BlueprintReadWrite)
	int32 score;
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

	/** TODO: maybe this method should be in the main cif subsystem because i think the method to
	 *	add objects in the world as cif game objects will be pretty much the same no matter how you
	 *	build your game
	 * 
	 * This method attaches to the caller's component the matching component that was initialized in the cif subsystem.
	 * For example, if a character was spawned in the level, it needs to be registered to the cif system to be taken
	 * into account in the social state
	 * @param objectName The object's name that we want to register as
	 * @param gameObjectCompRef The game object's component that will be filled with the matching component that was initialized in the CiF subsystem
	 * @return True if successfully found and initialized the component
	 */
	UFUNCTION(BlueprintCallable)
	bool registerAsGameObject(const FName objectName, UCiFGameObject*& gameObjectCompRef);
};
