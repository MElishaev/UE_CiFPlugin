// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CiFCharacter.h"
#include "CiFEffect.h"
#include "CiFSocialExchange.h"
#include "CiFSocialNetwork.h"
#include "UObject/Object.h"
#include "CiFManager.generated.h"

enum class EPredicateType : uint8;
class UCiFRuleRecord;
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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSocialNetworkUpdated, ESocialNetworkType, type);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRelationshipUpdated, ERelationshipType, type);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatusUpdated, EPredicateType, predType);


/**
 * 
 */
UCLASS(BlueprintType)
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

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSocialNetworkUpdated OnSocialNetworkUpdated;
	
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
	void formIntentForSocialGames(UCiFCharacter* initiator, UCiFGameObject* responder, const TArray<UCiFGameObject*>& possibleOthers = {});

	void formIntentForSpecificSocialExchange(UCiFSocialExchange* socialExchange,
	                                         UCiFCharacter* initiator,
	                                         UCiFGameObject* responder,
	                                         const TArray<UCiFGameObject*>& possibleOthers = {});

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
	                          UCiFGameObject* responder,
	                          const TArray<UCiFGameObject*>& possibleOthers = {});

	/* Scores all micro-theories for either initiator or responder */
	int8 scoreAllMicrotheoriesForType(UCiFSocialExchange* se,
	                                  UCiFCharacter* initiator,
	                                  UCiFGameObject* responder,
	                                  const TArray<UCiFGameObject*>& possibleOthers = {});

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
	 * (April)
	 * Called when needing to pick what effect and other the player wants
	 * @param outEffects	The output param that will be filled with all possible effects rather than just the most salient
	 * @param sg			The social game 
	 * @param isAccepted	Indicates if social game was accepted - TODO: but if it wasn't isn't there effects for this case also?
	 * @param initiator
	 * @param responder
	 * @param otherCast		The possible others
	 * @param levelCast		The others that are near (in the level or vicinity, depends on your game implementation and definition of this)
	 */
	void getAllSalientEffects(TArray<UCiFEffect*>& outEffects,
	                          UCiFSocialExchange* sg,
	                          const bool isAccepted,
	                          UCiFGameObject* initiator,
	                          UCiFGameObject* responder,
	                          const TArray<UCiFGameObject*> otherCast = {},
	                          TArray<UCiFGameObject*> levelCast = {});

	void changeSocialState(UCiFSocialExchangeContext* sgContext, TArray<UCiFGameObject*> otherCast = {});
	
	/**
	 * Figures out how important each predicate was in the initiator's desire to play a game
	 * @return A vector of influence rules where each rule has only one predicate and weight
	 * on the rule is the percent that the rule contributed to the initiator wanting to play that game
	 */
	TArray<UCiFRuleRecord*> getPredicateRelevance(UCiFSocialExchange* sg,
	                                              UCiFGameObject* initiator,
	                                              UCiFGameObject* responder,
	                                              UCiFGameObject* other = nullptr,
	                                              const FName forRole = "initiator",
	                                              TArray<UCiFGameObject*> otherCast = {},
	                                              const FName mode = "positive");

	
	/**
	 * Chooses a CKB object from those specified by the parameterized characters and CKBEntry predicate
	 * @param initiator the character in the initiator role
	 * @param responder the character in the responder role
	 * @param ckbPredicate the CKB entry type predicate holding the constraints on sought-after CKB objects
	 * @return The name of the chosen CKB object
	 */
	FName pickAGoodCKBObject(const UCiFGameObject* initiator, const UCiFGameObject* responder, const UCiFPredicate* ckbPredicate) const;

	/********************************** Getters ********************************/
	UCiFGameObject* getGameObjectByName(const FName name) const;
	UCiFItem* getItemByName(const FName name) const;
	UCiFKnowledge* getKnowledgeByName(const FName name) const;
	
	UFUNCTION(BlueprintCallable)
	UCiFSocialNetwork* getSocialNetworkByType(const ESocialNetworkType type) const;

	UCiFMicrotheory* getMicrotheoryByName(const FName mtName);
	
	void getAllGameObjects(TArray<UCiFGameObject*>& outGameObjs) const;
	void getAllGameObjectsNames(TArray<FName>& outObjNames) const;
	
	UFUNCTION(BlueprintCallable)
	void getAllGameObjectsOfType(TArray<UCiFGameObject*>& outGameObjs, const ECiFGameObjectType type) const;

	//TODO-fix bug where the type could be relationship but then we search it as social network and not relationship net
	int8 getNetworkWeightByType(const ESocialNetworkType netType, const uint8 id1, const uint8 id2) const;
private:

	void notifySocialStateChange(const UCiFEffect* effect);
	
	/* Clears all characters' prospective memory */
	void clearProspectiveMemory();

	/* Load the needed components and CiF state from json files.
	 * This is for new game initialization.
	 * For loading existing state, it is suggested to just load game from a
	 * serialized UE save game system. TODO - when load game will be supported later on
	 */
	void loadSocialGameLib(const FString& filePath, const UObject* worldContextObject);
	void loadMicrotheories(const FString& filePath, const UObject* worldContextObject);
	void loadCast(const FString& filePath, const UObject* worldContextObject);
	void loadItemList(const FString& filePath, const UObject* worldContextObject);
	void loadKnowledgeList(const FString& filePath, const UObject* worldContextObject);
	void loadCKB(const FString& filePath, const UObject* worldContextObject);
	void loadSFDB(const FString& filePath, const UObject* worldContextObject);
	void loadSocialNetworks(const FString& filePath, const UObject* worldContextObject);
	void loadPlotPoints(const FString& filePath, const UObject* worldContextObject);
	void loadQuestLib(const FString& filePath, const UObject* worldContextObject);
	void loadTriggers(const FString& filePath, const UObject* worldContextObject);

public:
	int32 mTime;

	UPROPERTY()
	UObject* mWorldContextObject;

	UPROPERTY(BlueprintReadOnly)
	UCiFCast* mCast;

	UPROPERTY()
	UCiFSocialExchangesLibrary* mSocialExchangesLib;

	UPROPERTY()
	TArray<UCiFItem*> mItemArray;

	UPROPERTY()
	TArray<UCiFKnowledge*> mKnowledgeArray;

	UPROPERTY()
	UCiFSocialFactsDataBase* mSFDB;

	UPROPERTY()
	UCiFCulturalKnowledgeBase* mCKB;

	UPROPERTY(BlueprintReadOnly)
	TMap<ESocialNetworkType, UCiFSocialNetwork*> mSocialNetworks;

	UPROPERTY()
	TMap<FName, UCiFMicrotheory*> mMicrotheoriesLib;

	UPROPERTY(BlueprintReadOnly)
	UCiFRelationshipNetwork* mRelationshipNetworks;

	/**
	 * this will always hold the last other that the last responder used while deciding accept/reject
	 * it should only be referenced immediately after play game
	 */
	UPROPERTY()
	UCiFGameObject* mLastResponderOther;
};
