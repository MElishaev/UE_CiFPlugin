// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CiFCharacter.h"
#include "UObject/Object.h"
#include "CiFManager.generated.h"

class UCiFRelationshipNetwork;
class UCiFSocialNetwork;
enum class ESocialNetworkType : uint8;
class UCiFMicrotheory;
class UCiFCulturalKnowledgeBase;
class UCiFSocialFactsDataBase;
class UCiFSocialExchange;
class UCiFSocialExchangesLibrary;
class UCiFCast;
/**
 * 
 */
UCLASS()
class CIF_API UCiFManager : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void formIntentForAll();

	/**
	 * Performs intent planning for a single character. This process scores
	 * all possible social games for all other characters and stores the 
	 * score in the character's prospective memory.
	 * 
	 * @param initiator The subject of the intent formation process.
	 */
	void formIntent(UCiFCharacter* initiator);

	/**
	 * Forms intent for all social games between two characters. If any of
	 * the social games requires a third party, it is handled properly. As
	 * with the other form intent functions, only intents that result in
	 * volition scores greater than 0 are added to the initiator's
	 * perspective memory.
	 * 
	 * @param initiator	The character in the initiator role.
	 * @param responder	The character in the responder role.
	 */
	void formIntentForSocialGames(UCiFCharacter* initiator, UCiFCharacter* responder, const TArray<UCiFCharacter*>& possibleOthers = {});

	void formIntentForSpecificSocialExchange(UCiFSocialExchange* socialExchange,
	                                         UCiFCharacter* initiator,
	                                         UCiFCharacter* responder,
	                                         const TArray<UCiFCharacter*>& possibleOthers = {});

	/**
	 * Forms the intents, each consisting of a gameScore, between an initiator, 
	 * responder,and an other determined by this function. The other is chosen
	 * first by permissibility (does he/she satisfy the preconditions of the
	 * game) and then by desirability (highest volition score). The chosen other
	 * is then placed along with the social game, initiator, and responder 
	 * in a game score which is stored in the initiator's perspective memory.
	 * 
	 * @param	socialExchange	The social game to contextualize the intent formation.
	 * @param	initiator		The character in the initiator role.
	 * @param	responder		The character in the responder role.
	 */
	void formIntentThirdParty(UCiFSocialExchange* socialExchange,
	                          UCiFCharacter* initiator,
	                          UCiFCharacter* responder,
	                          const TArray<UCiFCharacter*>& possibleOthers = {});

	/* Scores all micro-theories for either initiator or responder */
	int8 scoreAllMicrotheoriesForType(UCiFSocialExchange* se,
	                                  UCiFCharacter* initiator,
	                                  UCiFCharacter* responder,
	                                  const TArray<UCiFCharacter*>& possibleOthers = {});

	/* Getters */
	UCiFGameObject* getGameObjectByName(const FName name) const;
	UCiFItem* getItemByName(const FName name) const;
	UCiFKnowledge* getKnowledgeByName(const FName name) const;
	UCiFSocialNetwork* getSocialNetworkByType(const ESocialNetworkType type) const;

private:
	/* Clears all characters' prospective memory */
	void clearProspectiveMemory();

public:
	UPROPERTY()
	UCiFCast* mCast;

	UPROPERTY()
	UCiFSocialExchangesLibrary* mSocialExchangesLib;

	UPROPERTY()
	TArray<UCiFItem*> mItemArray;

	UPROPERTY()
	TArray<UCiFKnowledge*> mKnowledgeArray;

	UPROPERTY()
	UCiFSocialFactsDataBase* mSFDB;

	UCiFCulturalKnowledgeBase* mCKB;

	TArray<UCiFMicrotheory*> mMicrotheories;

	TMap<ESocialNetworkType, UCiFSocialNetwork*> mSocialNetworks;
	UCiFRelationshipNetwork* mRelationshipNetworks;
};
