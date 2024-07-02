// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CiFCharacter.h"
#include "CiFEffect.h"
#include "CiFSocialExchange.h"
#include "UObject/Object.h"
#include "CiFManager.generated.h"

class UCiFPredicate;
class UCiFEffect;
class UCiFSocialExchangeContext;
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

	UCiFManager();
	
	/**
	 * @param worldContextObject	world context for the ability to access the game
	 *								instance and CiF subsystem in all of the classes inside CiF module
	 *								see https://forums.unrealengine.com/t/how-to-get-the-current-world-from-a-blueprint/401898/3?u=dakrn1k3
	 *								for explanation about this meta parameter, but overall the caller of this init
	 *								from inside the game sends itself (*this) as the world context object
	 *								such that he is the one "knowing" about the world he is in
	 */
	UFUNCTION(BlueprintCallable, meta = (WorldContext="WorldContextObject"))
	void init(const UObject* worldContextObject);
	
	void parseSocialGameLib(const FString& filePath, const UObject* worldContextObject);
	
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

	UCiFSocialExchangeContext* playGame(UCiFSocialExchange* sg,
	                                    UCiFGameObject* initiator,
	                                    UCiFGameObject* responder,
	                                    UCiFGameObject* other = nullptr,
	                                    TArray<UCiFGameObject*> otherCast = {},
	                                    TArray<UCiFGameObject*> levelCast = {},
	                                    UCiFEffect* chosenEffect = nullptr);

	float getResponderScore(UCiFSocialExchange* sg,
	                        UCiFGameObject* initiator,
	                        UCiFGameObject* responder,
	                        const TArray<UCiFGameObject*>& activeOtherCast = {});

	/**
	 * Returns a third character that makes the highest number of effect
	 * condition predicates evaluate true.
	 * @param	outOther	The output param for the third character
	 * @param	outEffect	The output param for the chosen effect
	 * @param	sg			The social game to reason over.
	 * @param 	isSgAccepted	Whether the game was accepted or rejected
	 * @param	initiator	The character in the initiator role.
	 * @param	responder	The character in the responder role.
	 * @return	The most salient other.
	 */
	void getSalientOtherAndEffect(UCiFGameObject*& outOther,
	                              UCiFEffect*& outEffect,
	                              UCiFSocialExchange* sg,
	                              const bool isSgAccepted,
	                              UCiFGameObject* initiator,
	                              UCiFGameObject* responder,
	                              const TArray<UCiFGameObject*>& otherCast = {},
	                              TArray<UCiFGameObject*> levelCast = {});

	/**
	 * Chooses a CKB object from those specified by the parameterized characters and CKBEntry predicate
	 * @param initiator the character in the initiator role
	 * @param responder the character in the responder role
	 * @param ckbPredicate the CKB entry type predicate holding the constraints on sought-after CKB objects
	 * @return The name of the chosen CKB object
	 */
	FName pickAGoodCKBObject(const UCiFGameObject* initiator, const UCiFGameObject* responder, const UCiFPredicate* ckbPredicate) const;
	
	/* Getters */
	UCiFGameObject* getGameObjectByName(const FName name) const;
	UCiFItem* getItemByName(const FName name) const;
	UCiFKnowledge* getKnowledgeByName(const FName name) const;
	UCiFSocialNetwork* getSocialNetworkByType(const ESocialNetworkType type) const;

private:
	/* Clears all characters' prospective memory */
	void clearProspectiveMemory();

public:

	int32 mTime;
	
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

	/**
	 * this will always hold the last other that the last responder used while deciding accept/reject
	 * it should only be referenced immediately after play game
	 */
	UCiFGameObject* mLastResponderOther;
};
