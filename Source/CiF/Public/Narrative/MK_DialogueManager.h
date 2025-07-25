// MAAKU Studio all rights reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Narrative/MK_NarrativeDataTypes.h"
#include "MK_DialogueManager.generated.h"


class UCifInstantiation;

/**
 * 
 */
UCLASS()
class CIF_API UMK_DialogueManager : public UObject
{
    GENERATED_BODY()

public:
    
    /**
     * Loads the registry of the game's dialogue that helps with faster and easier parsing and
     * lookup when a dialogue needs to be loaded from disk.
     * This helps to separate the dialogue of the game to multiple files for easier managing,
     * and load and parsing time when dialogue needs to be pulled from a file.
     * @return true if successfully parsed the registry and loaded into @mDialogueRegistry
     */
    bool initializeRegistry(const FString& registryPath);

    /**
     * Loads the dialogue if not already loaded and update the @mCurrentDialogueID with the requested dialogue.
     * This must be called before requesting text from the dialogue
     * @param dialogueID id for the requested dialogue
     */
    void prepareDialogue(const FName dialogueID);

    /**
     * Clears all loaded dialogues (if any) with IDs that starts with prefix.
     * Could be used to remove all plot points by prefix=pp_, or plot points specifically related to a character
     * prefix=pp_charName etc.
     */
    void clearDialoguesWithPrefix(const FString& prefix);

    TArray<FDialogueChoice> getChoices() const;
    void selectChoice(int32 choiceIndex);
    
private:
    
    // Load dialogues from a specific file
    bool loadDialogueFile(const FString& filePath);

    FString findDialogueFile(const FName dialogueID) const;

    bool areChoicesConditionsMet(const TArray<FDialogueCondition>& conditions) const;
    
    TMap<FString, FDialogueFileEntry> mDialogueRegistry; // Maps prefix to file
    TMap<FName, UCifInstantiation*> mLoadedDialogues;         // Cached dialogues

    /****** Members of current ongoing dialogue ******/
    FName mCurrentDialogueID;
    uint16_t mCurrentDialogueLineNum = 0;
};
