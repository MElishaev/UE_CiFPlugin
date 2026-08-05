
#pragma once

#include "CoreMinimal.h"
#include "MK_NarrativeDataTypes.generated.h"


// Registry entry for dialogue files (this object doesn't need to be serialized so UE macros removed)
struct FDialogueFileEntry
{
    FName mInstantiationId; // Globally unique ID used by narrative systems.
    FString mFilePath;      // Path relative to the registry file.
};

// Represents a condition for showing a choice
USTRUCT()
struct FDialogueCondition
{
    GENERATED_BODY()

    UPROPERTY()
    FString type; // e.g., "flag_set" // todo can be changed to enum instead of comparing strings
    UPROPERTY()
    FString flag; // Flag name
    UPROPERTY()
    FString value; // Expected value
};

// Represents a player choice
USTRUCT()
struct FDialogueChoice
{
    GENERATED_BODY()

    UPROPERTY()
    FName textKey; // Localization key for choice text
    UPROPERTY()
    FName nextID; // Next dialogue node ID
    UPROPERTY()
    TArray<FDialogueCondition> conditions; // Conditions to show todo maybe should be predicate
};

/** One line emitted by dialogue playback, including the character who speaks it. */
USTRUCT(BlueprintType)
struct FDialogueLine
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "CiF|Narrative")
    FName mSpeaker = NAME_None;

    UPROPERTY(BlueprintReadOnly, Category = "CiF|Narrative")
    FText mText;
};

/** Represents a dialogue node, which could consist many dialogue lines of the same character
 * that relate to each other contextually -
 * this means that if few dialogue boxes are said by the same character with the same emotion/animation, then they may be grouped together
 * because they have common data (for now i don't have animation and emotion represented in the dialogue node)
 */
USTRUCT()
struct FDialogueNode
{
    GENERATED_BODY()

    UPROPERTY()
    FName id; // Unique identifier for traversing dialogue tree
    UPROPERTY()
    FName speaker; // Speaker name
    UPROPERTY()
    TArray<FText> text; // Text represent dialogue boxes
    UPROPERTY()
    TArray<FDialogueChoice> choices; // Player choices
};
