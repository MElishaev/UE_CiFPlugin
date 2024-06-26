// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CiFSFDBContext.h"
#include "CiFSocialFactsDataBase.h"
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

public:
	FName mInitiatorName; // name of the initiator of this social exchange
	FName mResponderName; // name of the responder of this social exchange
	bool mIsBackstory;
	FSFDBLabel mSFDBLabel; // the SFDB label for this context entry

	// todo - why is there many labels associated with this context entry? i need an example to understand what does it mean
	TArray<FSFDBLabel> mSFDBLabels; // the SFDB labels associated with this context entry 
};
