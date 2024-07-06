// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CiFSFDBContext.h"
#include "CiFSocialFactsDataBase.h"
#include "Utilities.h"
#include "CiFSocialExchangeContext.generated.h"

class UCiFGameObject;
enum class ESFDBLabelType : uint8;
/**
 * This class is where the details of a played social game
 * (known as a historical context) are kept. Contexts are stored in the 
 * SFDB for use by characters in the future.
 *
 * Example:
 * <SFDB>
 *   <SocialGameContext gameName="conversational flirt" initiator="edward" responder="karen"
 * 		other="" initiatorScore="80" responderScore="40" time="4" effect="2" chosenCKBItem="black nail polish"
 * 		socialGameContextReference="2">
 * 			<Predciate type="cbk" .../>
 *   </SocialGameContext>
 *   <TriggerContext status="cheating" from="edward" to="karen">
 * </SFDB>
 * 
 */
UCLASS()
class CIF_API UCiFSocialExchangeContext : public UCiFSFDBContext
{
	GENERATED_BODY()

public:

	virtual ESFDBContextType getType() const override;
	
	/**
	 * This one is used with numTimesUniquelyTrue sfdb label predicates
	 * 
	 * Determines if the passed-in parameters of label and characters match the SFDBLabel and
	 * characters related to the label in this SocialGameContext. If this context is a backstory
	 * context, the first and second character parameters must match the context's initiator and
	 * responder respectively. If it is a non-backstory context, the first and second character
	 * parameters must match the labelArg1 and labelArg2 properties respectively.
	 * 
	 * @param	labelType			The label to match. If INVALID is passed in, all labels are considered to match.
	 * @param	first				The first character parameter.
	 * @param	second				The second character parameter.
	 * @param	third				The third character parameter (not currently used).
	 * @return	True if this context is a match to the parameters. False if not a match.
	 */
	bool doesSFDBLabelMatchStrict(const ESFDBLabelType labelType,
	                              const UCiFGameObject* first,
	                              const UCiFGameObject* second = nullptr,
	                              const UCiFGameObject* third = nullptr,
	                              const UCiFPredicate* pred = nullptr) const;

	/**
	 * Determines if the passed-in parameters of label and characters match the SFDBLabel and
	 * characters related to the label in this SocialGameContext. If this context is a backstory
	 * context, the first and second character parameters must match the context's initiator and
	 * responder respectively. If it is a non-backstory context, the first and second character
	 * parameters must match the labelArg1 and labelArg2 properties respectively.
	 * 
	 * @param	labelType	The label to match. If -1 is passed in, all labels are considered to match.
	 * @param	first		The first character parameter.
	 * @param	second		The second character parameter.
	 * @param	third		The third character parameter (not currently used).
	 * @return	True if this context is a match to the parameters. False if not a match.
	 */
	bool doesSFDBLabelMatch(const ESFDBLabelType labelType,
	                        const UCiFGameObject* first,
	                        const UCiFGameObject* second = nullptr,
	                        const UCiFGameObject* third = nullptr,
	                        const UCiFPredicate* pred = nullptr) const;

	static UCiFSocialExchangeContext* loadFromJson(const TSharedPtr<FJsonObject> json, const UObject* worldContextObject);
public:
	FName mGameName; // the social game name
	FName mChosenItemCKB; // the name of the item brought up in the performance realization that involves a CKB entry
	FName mInitiatorName; // name of the initiator of this social exchange
	FName mResponderName; // name of the responder of this social exchange
	FName mOtherName; // name of the other of this social exchange (if any)
	int8 mInitiatorScore; // score of initiator IRS
	int8 mResponderScore; // score of responder IRS
	bool mIsBackstory;
	FSFDBLabel mSFDBLabel; // the SFDB label for this context entry
	IdType mEffectId; // not sure yet what is this - i think it may be the effect ID that is the result of this social exchange
	FString mPerformanceRealization; // this describes what happened in this social game in an NLG

	// todo - why is there many labels associated with this context entry? i need an example to understand what does it mean
	TArray<FSFDBLabel> mSFDBLabels; // the SFDB labels associated with this context entry 
};
