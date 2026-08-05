// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Narrative/MK_NarrativeDataTypes.h"
#include "UObject/Object.h"
#include "Utilities.h"
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

    /** Advances playback and writes the next line, including its speaker. Returns false when playback is complete. */
    bool getNextDialogueLine(FDialogueLine& outLine);

    /** Rewinds dialogue playback to its first line. */
    void resetDialogue();

    /** Returns true when no dialogue text remains after the current playback position. */
    bool isDialogueFinished() const;

    FORCEINLINE ID_t getmId() const { return mId; }
    FORCEINLINE FName getName() const { return mName; }
    FText getmDescription(const UCiFGameObject* initiator, const UCiFGameObject* responder, const UCiFGameObject* other) const;

    bool requiresOtherToPerform() const;

    static UCifInstantiation* loadFromJson(const TSharedPtr<FJsonObject> json, UObject* worldContextObj);

    /** Loads one complete dialogue-backed instantiation from a standalone file's root JSON object. */
    static UCifInstantiation* loadDialogueFromJson(const TSharedPtr<FJsonObject> json, UObject* worldContextObj);

private:
    /**
     * this method takes the line as written in the json file and swaps the in-line variables (like %r% etc.)
     * with actual names based on the social game context
     */
    FText realizeDialogueLine(const UCiFGameObject* initiator, const UCiFGameObject* responder, const UCiFGameObject* other) const;

    FString extractSubstringBetweenDelimiter(const FString& srcStr,
                                             const FString& openingDelimiter,
                                             const FString& closingDelimiter,
                                             int32 startIndex = 0,
                                             const bool includingDelimiter = true) const;
    FString replaceInlineDialogueOperators(const FString& inStr,
                                           const UCiFGameObject* initiator,
                                           const UCiFGameObject* responder,
                                           const UCiFGameObject* other) const;

    UPROPERTY(VisibleAnywhere, Category = "CiF|Narrative")
    TArray<FDialogueNode> mDialogueNodes;

    int16 mCurrentNode = 0;
    int16 mCurrentLine = 0;
    FText mSrcDescription;     // description from the json
    ID_t mId = CIF_INVALID_ID; // legacy numeric ID used by embedded social-game effects
    FName mName;               // name of this instantiation
};
