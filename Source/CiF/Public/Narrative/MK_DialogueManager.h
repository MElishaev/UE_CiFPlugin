// MAAKU Studio all rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Narrative/MK_NarrativeDataTypes.h"
#include "UObject/Object.h"
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
     * Loads an instantiation if it is not already cached and makes it the current dialogue.
     * @param instantiationId exact ID
     * registered for the requested instantiation
     * @return the cached or newly loaded instantiation, or nullptr when it cannot be
     * resolved
     */
    UCifInstantiation* prepareDialogue(const FName instantiationId);

    /**
     * Clears all loaded dialogues (if any) with IDs that starts with prefix.
     * Could be used to remove all plot points by prefix=pp_, or plot points specifically related to a character
     * prefix=pp_charName etc.
     */
    void clearDialoguesWithPrefix(const FString& prefix);

    TArray<FDialogueChoice> getChoices() const;
    void selectChoice(int32 choiceIndex);

private:
    // Load one complete dialogue-backed instantiation from a specific file.
    bool loadDialogueFile(const FString& filePath);

    FString findDialogueFile(const FName instantiationId) const;

    bool areChoicesConditionsMet(const TArray<FDialogueCondition>& conditions) const;

    TMap<FName, FDialogueFileEntry> mDialogueRegistry; // Maps an exact instantiation ID to its file.
    TMap<FName, UCifInstantiation*> mLoadedDialogues;  // Cached instantiations.

    FString mRegistryDirectory;

    /****** Members of current ongoing dialogue ******/
    FName mCurrentDialogueID;
    uint16_t mCurrentDialogueLineNum = 0;
};
