﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CiFEffect.h"
#include "CifImplementationBase.h"
#include "DemoCifImplementation.generated.h"

class UCiFCharacter;
class UCiFSocialExchangeContext;
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
	/**
	 * Chooses an initiator for a social game.
	 * (this is based on the game logic for how to choose an initiator, maybe at random, maybe the next in queue etc.
	 * for now, this is just iterating in a sequence over the cast of characters which are not player)
	 * @return the chosen initiator
	 */
	UFUNCTION(BlueprintCallable)
	UCiFCharacter* chooseNPCInitiatorForSocialGame();
	
	// TODO - maybe change this name later - this is seem to be more related to when the player engages a SG
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


	/**
	 * Allows to select what other objects to discuss, share or talk about (characters/items/knowledge).
	 * this seem to be called in any case of choosing any social move (even if others aren't needed for
	 * the social move), when no other is needed, it is directly calls moveChosen, which I'm not sure
	 * I like this approach. I would rather it being a separate method that only called to offer others
	 * 
	 * @param outOthers Output array holds possible others for this social game
	 * @param sgName Social game name
	 * @param initiator The initiator of the social game
	 * @param responder Responder of the social game
	 * @param isNPC True iff the initiator is an NPC
	 */
	UFUNCTION(BlueprintCallable)
	void offerOthers(TArray<UCiFGameObject*>& outOthers,
	                 const FName sgName,
	                 ACifNPC* initiator,
	                 const FName responder,
	                 bool isNPC);

	/** 
	 * Only should be called after offerOthers().
	 * This method calls the moveChosen with the other that was chosen.
	 * If it is a move of type USE ITEM or GIVE ITEM etc, it calls offerEffects instead
	 */
	UFUNCTION(BlueprintCallable)
	void otherChosen(ACifNPC* initiator, UCiFGameObject* responder, UCiFGameObject* other, const FName sgName, const bool isNPC);

	/**
	 * Should be called when the move requires choosing an effect (Give Gift, Give Romantic Gift, Use)
	 * Allows the player to select what specific effect they want to enact with a move
	 * otherChosen signals the use of this inside a character move (instead of an item)
	 */
	UFUNCTION(BlueprintCallable)
	void offerEffects(TArray<UCiFEffect*>& outEffects, const FName sgName, ACifNPC* initiator, UCiFGameObject* responder, UCiFGameObject* otherChosen = nullptr);

	void itemMoveChosen(const FName sgName, ACifNPC* initiator, UCiFGameObject* responder, const bool isNPCPlaying, UCiFEffect* effect);
	
	// TODO - what this function returns?
	UFUNCTION(BlueprintCallable)
	void moveChosen(const FName sgName,
	                ACifNPC* initiator,
	                UCiFGameObject* responder,
	                bool isNPC,
	                UCiFGameObject* other = nullptr,
	                UCiFEffect* effect = nullptr);

	// Only should be called after offerOthers()
	// e is the effect.referenceAsNaturalLanguage
	UFUNCTION(BlueprintCallable)
	void effectChosen(const FName sgName, ACifNPC* initiator, UCiFGameObject* responder, const bool isNPC, UCiFEffect* effect, UCiFGameObject* other);

	/**
	 * Should be called in any situation where social moves including items will be played
	 * This includes moves with items (Pick up, Put down)
	 * AND moves with characters concerning items (Give item)
	 * Please note the order of predicates in the effect rules matters for the game state to change correctly
	 */
	void handleItemMoveEffects(UCiFSocialExchangeContext* context);

private:
	uint8_t mCharacterIndexInCast = 0; // this is used to choose the next initiator for a social game
};
