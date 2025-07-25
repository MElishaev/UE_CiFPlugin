// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Utilities.h"
#include "UObject/Object.h"
#include "Narrative/MK_NarrativeDataTypes.h"
#include "CifInstantiation.generated.h"

class UCiFGameObject;

/**
 * 
 */
UCLASS()
class CIF_API UCifInstantiation : public UObject
{
    GENERATED_BODY()

public:

    void init(const int id, const FName name, const FText& description);
    
    /**
     * @return array of possible choices if any available for the current line, else return empty array
     */
    void getChoicesIfAvailable(TArray<FDialogueChoice>& outChoices) const;

    /**
     * @return FText representing the next dialogue line in the current dialogue
     * todo - what if not valid dialogue running?
     */
    FText getNextDialogueLine();

    FORCEINLINE ID_t getmId() const { return mId; }
    FText getmDescription(const UCiFGameObject* initiator, const UCiFGameObject* responder, const UCiFGameObject* other) const;

    bool requiresOtherToPerform() const;
    
    static UCifInstantiation* loadFromJson(const TSharedPtr<FJsonObject> json, UObject* worldContextObj);
   
private:
    /**
     * @return formatted text for the current dialogue line
     */
    FText getCurrentLine() const;

    /**
     * this method takes the line as written in the json file and swaps the in-line variables (like %r% etc.)
     * with actual names based on the social game context
     */
    FText realizeDialogueLine(const UCiFGameObject* initiator, const UCiFGameObject* responder, const UCiFGameObject* other) const;
    
    int16 mCurrentLine = 0; // current line of the dialogue
    FText mSrcDescription;  // description from the json
    ID_t mId;                // id from the json
    FName mName;            // name of this instantiation
};
