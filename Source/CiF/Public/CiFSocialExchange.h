// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Utilities.h"
#include "CiFSocialExchange.generated.h"

enum class ECiFGameObjectType : uint8;
class UCiFInfluenceRuleSet;
class UCiFInstantiation;
class UCiFEffect;
class UCiFCharacter;
class UCiFRule;
class UCiFGameObject;

UENUM(BlueprintType)
enum class ESocialExchangeIntent : uint8
{
	INCREASE,
	// could be to increase some stats between characters
	DECREASE,
	START,
	// could be to start some relationship/friendship 
	END
};

/**
 * The SocialGame class stores social games in their declarative form; the
 * social game preconditions, influence sets, and effects are stored. What
 * is not stored is the decisions made by the player or AI system about what
 * game choices are made. These decisions are stored in the FilledGame class.
 */
UCLASS()
class CIF_API UCiFSocialExchange : public UObject
{
	GENERATED_BODY()

public:
	/* Adds an effect to the effects list and gives it an ID. */
	void addEffect(UCiFEffect* effect);
	void addInstantiation(UCiFInstantiation* instantiation);

	/* returns pointer to the effect or nullptr if doesn't exist */
	UCiFEffect* getEffectById(const uint32 id);
	UCiFInstantiation* getInstantiationById(const uint32 id);

	/**
	 * Returns the initiator's influence rule set score with respect to the
	 * character/role mapping given in the arguments.
	 * 
	 * @param	initiator	The initiator of the social game.
	 * @param	responder	The responder of the social game.
	 * @param	other		A third party in the social game.
	 */
	float getInitiatorScore(UCiFCharacter* initiator,
	                        UCiFGameObject* responder,
	                        UCiFGameObject* other = nullptr,
	                        UCiFSocialExchange* se = nullptr,
	                        TArray<UCiFGameObject*> activeOtherCast = {});

	/**
	 * Returns the responder's influence rule set score with respect to the
	 * character/role mapping given in the arguments.
	 * 
	 * @param	initiator	The initiator of the social game.
	 * @param	responder	The responder of the social game.
	 * @param	other		A third party in the social game.
	 */
	float getResponderScore(UCiFCharacter* initiator,
	                        UCiFGameObject* responder,
	                        UCiFGameObject* other = nullptr,
	                        UCiFSocialExchange* se = nullptr,
	                        TArray<UCiFGameObject*> activeOtherCast = {});

	/**
	 * This function will score an influence rule set for all others that fit the definition or no others 
	 * if the definition doesn't require it. This function assumes that there is one precondition rule.
	 * 
	 * @param	initiator
	 * @param	responder
	 * @param	activeOtherCast
	 * @return The total weight of the influence rules
	 */
	float scoreSocialExchange(UCiFCharacter* initiator,
	                          UCiFGameObject* responder,
	                          TArray<UCiFGameObject*> activeOtherCast = {},
	                          bool isResponder = false);


	/**
	 * Evaluates the preconditions of the social game with respect to 
	 * character/role mapping given in the arguments for the initiator
	 * and responder while finding an other that fits all precondition 
	 * rules if a third character is require by any of those rules.
	 * Upon the first @other that will be found the method will return true.
	 * 
	 * @param	initiator		The initiator of the social game.
	 * @param	responder		The responder of the social game.
	 * @param 	activeOtherCast The possible characters to fill the other role.
	 * @return True if all precondition rules evaluate to true. False if 
	 * they do not.
	 */
	bool checkPreconditionsVariableOther(UCiFCharacter* initiator, UCiFGameObject* responder, TArray<UCiFGameObject*> activeOtherCast = {});

	/**
	 * Evaluates the preconditions of the social game with respect to 
	 * character/role mapping given in the arguments.
	 * 
	 * @param    initiator    The initiator of the social game.
	 * @param    responder    The responder of the social game.
	 * @param    other        A third party in the social game.
	 * @return True if all precondition rules evaluate to true. False if 
	 * they do not.
	 */
	bool checkPreconditions(UCiFCharacter* initiator,
	                        UCiFGameObject* responder,
	                        UCiFGameObject* other = nullptr,
	                        UCiFSocialExchange* se = nullptr);

	/**
	 * Evaluates the intents of the social game with respect to 
	 * character/role mapping given in the arguments.
	 * 
	 * @param	initiator	The initiator of the social game.
	 * @param	responder	The responder of the social game.
	 * @param	other		A third party in the social game.
	 * @return True if all intent rules evaluate to true. False if 
	 * they do not.
	 */
	bool checkIntents(UCiFCharacter* initiator,
	                  UCiFGameObject* responder,
	                  UCiFGameObject* other = nullptr,
	                  UCiFSocialExchange* se = nullptr);
	
	/**
	 * Determines if we need to find a third character for the intent
	 * formation process.
	 * @return True if a third character is needed, false if not.
	 */
	bool isThirdNeededForIntentFormation();

	/** todo - don't understand the different between this and @isThirdNeededForIntentFormation
	 * Determines if we need to find a third character for social game 
	 * play.
	 * @return True if a third character is needed, false if not.
	 */
	bool isThirdForSocialExchangePlay();

	// todo - what is the difference between this and the above?
	bool isThirdParty() const { return mIsTalkAboutSomeone || mIsGetSomeoneToDoSomethingForYou; }
	
	/* this method is more appropriate for mismanor than prom week because in mismanor you have items which
	 * are also game objects, and because of that you need to check that the other is viable for social interactions
	 * to make sense. for example, it wouldn't make sense if the interaction is CharacterA wants to give CharacterB
	 * an CharacterC. CharacterC is not a possible other, because in this case the possible other for this social
	 * exchange should be ITEMS only.
	 */
	TArray<UCiFGameObject*> getPossibleOthers(UCiFGameObject* initiator, UCiFGameObject* responder);

public:
	FName mName;
	TArray<UCiFRule*> mIntents;
	TArray<UCiFRule*> mPreconditions; // specify under which conditions any given social exchange is possible
	// Initiator influence rules are used to determine the volition (desire) for a character to initiate a social exchange with other characters
	UCiFInfluenceRuleSet* mInitiatorIR;
	// Responder influence rules are used to determine whether a responder accepts or rejects the social exchange
	UCiFInfluenceRuleSet* mResponderIR;
	TArray<UCiFEffect*> mEffects;               // the effects of the social exchange
	TArray<UCiFInstantiation*> mInstantiations; // the realization of the outcome of the this social exchange
	UCiFRule* mPatsyRule;                       // TODO - no idea what this is
	bool mIsTalkAboutSomeone;
	bool mIsGetSomeoneToDoSomethingForYou;

	bool mIsRequiresOther; // todo -maybe delete

	ECiFGameObjectType mOtherType;
	
protected:
	IdType Id;        // todo -maybe delete
	FName mInitiator; // todo -maybe delete
	FName mResponder; // todo -maybe delete
	FName mOther;     // if a third party is involved // todo -maybe delete


	/**
	* initiator’s purpose for initiating the social exchange. These 
	* purposes involve changing social state, such as increasing another character’s friendship 
	* feelings towards the initiator (a social network value change) or initiating or terminating 
	* a relationship (such as a dating relationship) etc.
	 */
	ESocialExchangeIntent mIntentType; // todo -maybe delete
};
