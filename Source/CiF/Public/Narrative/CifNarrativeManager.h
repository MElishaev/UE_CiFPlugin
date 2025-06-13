// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CifNarrativeManager.generated.h"

class UCifPlotPointPool;
class UCifInstantiation;

// Registry entry for dialogue files (this object doesn't need to be serialized so UE macros removed)
struct FDialogueFileEntry
{
    FString dialogueIDPrefix; // e.g., "pp_colonel_"
    FString filePath;         // e.g., "Content/Data/Dialogue/pp/NPC1.json"
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

/** Represents a dialogue node, which could consist many dialogue lines of the same character
 * that relate to each other contextually - this means that if few dialogue boxes are said by
 * the same character with the same emotion/animation, then they may be grouped together because
 * they have common data (for now i don't have animation and emotion represented in the dialogue node)
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

/**
 * 
 */
UCLASS()
class CIF_API UCifNarrativeManager : public UObject
{
    GENERATED_BODY()

public:

    void init();
    
    // todo should i leave the instantiation? do i need this? i need to remember that i will also need
    // at some point to populate the text based on different specifiers in the text, like %r %i etc.
    // this could have a nice amount of logic that can pollute more main classes like this one.
    
    UCifInstantiation* getInstantiation(FName dlgContext) const { return mInstantiationsMap.FindRef(dlgContext); }

    // todo maybe extract the dialogue related methods into a dialogue manager class?

    /**
     * While in a social game that is playing out, check if the responder in the social game has a plot point
     * to reveal to the player. Return the plot point ID to be played
     * @param responder the responder in the current playing SG
     * @return plot point name or NONE if no plot point to play
     * todo but why would we want the pp name to be returned? y not directly start playing the pp?
     * just do get dialogue if pp available to be played and thats it.
     */
    FName getPPNameToBePlayed(const FName responder) const;

    /**
     * Loads the dialogue if not already loaded and update the @mCurrentDialogueID with the requested dialogue.
     * This must be called before requesting text from the dialogue
     * @param dialogueID id for the requested dialogue
     */
    void prepareDialogue(const FName& dialogueID);

    void clearDialogues(const FString& prefix);

    FText getNextDialogueLine();

    TArray<FDialogueChoice> getChoices() const;
    void selectChoice(int32 choiceIndex);


private:

    void loadPlotPoints(const FString& filePath, const UObject* worldContextObject);
    
    FText getCurrentText() const;
    
    // Load dialogues from a specific file
    bool loadDialogueFile(const FString& filePath);

    /**
     * Loads the registry of the game's dialogue that helps with faster and easier parsing and
     * lookup when a dialogue needs to be loaded from disk.
     * This helps to separate the dialogue of the game to multiple files for easier managing,
     * and load and parsing time when dialogue needs to be pulled from a file.
     * @return true if successfully parsed the registry and loaded into @mDialogueRegistry
     */
    bool initializeRegistry(const FString& registryPath);

    FString findDialogueFile(const FName dialogueID) const;

    bool areChoicesConditionsMet(const TArray<FDialogueCondition>& conditions) const;

public:
    UPROPERTY()
    UCifPlotPointPool* mPlotPointPool;

    // todo - this might be not necessary and maybe we will initialize based on need instead of all from the get go.
    // todo - some instantiation are not needed after already played.. like used dialogue and plot points which will never be replayed
    UPROPERTY()
    TMap<FName, UCifInstantiation*> mInstantiationsMap;

private:
    TMap<FName, FDialogueNode> mLoadedDialogues;         // Cached dialogues
    TMap<FString, FDialogueFileEntry> mDialogueRegistry; // Maps prefix to file
    FName mCurrentDialogueID;
    uint16_t mCurrentDialogueLineNum = 0;
};
