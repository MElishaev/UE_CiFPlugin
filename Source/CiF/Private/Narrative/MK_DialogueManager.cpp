// MAAKU Studio all rights reserved


#include "Narrative/MK_DialogueManager.h"

#include "ReadWriteFiles.h"
#include "Narrative/CifInstantiation.h"

bool UMK_DialogueManager::initializeRegistry(const FString& registryPath)
{
    TSharedPtr<FJsonObject> jsonObject;
    if (!UReadWriteFiles::readJson(registryPath, jsonObject)) {
        UE_LOG(LogTemp, Error, TEXT("Failed to read json %s"), *registryPath);
        return false;
    }

    mDialogueRegistry.Empty();
    const TArray<TSharedPtr<FJsonValue>>* filesArray;
    if (jsonObject->TryGetArrayField(TEXT("dialogue_files"), filesArray)) {
        for (const TSharedPtr<FJsonValue>& fileElem : *filesArray) {
            const TSharedPtr<FJsonObject>& fileObj = fileElem->AsObject();
            FDialogueFileEntry entry;
            entry.dialogueIDPrefix = fileObj->GetStringField(TEXT("dialogue_id_prefix"));
            entry.filePath = fileObj->GetStringField(TEXT("file_path"));
            mDialogueRegistry.Add(entry.dialogueIDPrefix, entry);
        }
    }
    return true;
}

void UMK_DialogueManager::prepareDialogue(const FName dialogueID)
{
    // Check if dialogue is already loaded
    if (mLoadedDialogues.Contains(dialogueID)) {
        mCurrentDialogueID = dialogueID;
        return;
    }

    const FString filePath = findDialogueFile(dialogueID);
    if (filePath.IsEmpty()) {
        return;
    }

    if (loadDialogueFile(filePath)) {
        if (mLoadedDialogues.Contains(dialogueID)) {
            mCurrentDialogueID = dialogueID;
        }
    }
}

void UMK_DialogueManager::clearDialoguesWithPrefix(const FString& prefix)
{
    for (auto it = mLoadedDialogues.CreateIterator(); it; ++it) {
        if (it.Key().ToString().StartsWith(prefix)) {
            it.RemoveCurrent();
        }
    }
}

TArray<FDialogueChoice> UMK_DialogueManager::getChoices() const
{
    TArray<FDialogueChoice> availableChoices;
    if (mLoadedDialogues.Contains(mCurrentDialogueID)) {
        TArray<FDialogueChoice> dlgChoices;
        mLoadedDialogues[mCurrentDialogueID]->getChoicesIfAvailable(dlgChoices);
        for (const FDialogueChoice& choice : dlgChoices) {
            if (areChoicesConditionsMet(choice.conditions)) {
                availableChoices.Add(choice);
            }
        }
    }
    return availableChoices;
}

void UMK_DialogueManager::selectChoice(int32 choiceIndex)
{
    if (mLoadedDialogues.Contains(mCurrentDialogueID)) {
        TArray<FDialogueChoice> dlgChoices;
        mLoadedDialogues[mCurrentDialogueID]->getChoicesIfAvailable(dlgChoices);
        if (choiceIndex >= 0 && choiceIndex < dlgChoices.Num() && areChoicesConditionsMet(dlgChoices[choiceIndex].conditions)) {
            mCurrentDialogueID = dlgChoices[choiceIndex].nextID;
        }
    }
}

bool UMK_DialogueManager::loadDialogueFile(const FString& filePath)
{
    TSharedPtr<FJsonObject> jsonObject;
    if (!UReadWriteFiles::readJson(filePath, jsonObject)) {
        UE_LOG(LogTemp, Error, TEXT("Failed to read json %s"), *filePath);
        return false;
    }

    const TArray<TSharedPtr<FJsonValue>>* dialoguesArray;
    if (jsonObject->TryGetArrayField(TEXT("dialogues"), dialoguesArray)) {
        for (const TSharedPtr<FJsonValue>& dialogueValue : *dialoguesArray) {
            const TSharedPtr<FJsonObject>& dialogueObj = dialogueValue->AsObject();
            FDialogueNode node;
            node.id = FName(dialogueObj->GetStringField(TEXT("id")));
            node.speaker = FName(dialogueObj->GetStringField(TEXT("speaker")));
            TArray<FString> dlgLines;
            dialogueObj->TryGetStringArrayField(TEXT("text"), dlgLines);
            for (const auto& line : dlgLines) {
                node.text.Add(FText::FromString(line));
            }

            // parse choices
            const TArray<TSharedPtr<FJsonValue>>* choicesArray;
            if (dialogueObj->TryGetArrayField(TEXT("choices"), choicesArray)) {
                for (const TSharedPtr<FJsonValue>& choiceValue : *choicesArray) {
                    const TSharedPtr<FJsonObject>& choiceObj = choiceValue->AsObject();
                    FDialogueChoice choice;
                    choice.textKey = FName(choiceObj->GetStringField(TEXT("text_key")));
                    choice.nextID = FName(choiceObj->GetStringField(TEXT("next_id")));

                    // parse choice conditions
                    const TArray<TSharedPtr<FJsonValue>>* conditionsArray;
                    if (choiceObj->TryGetArrayField(TEXT("conditions"), conditionsArray)) {
                        for (const TSharedPtr<FJsonValue>& conditionValue : *conditionsArray) {
                            const TSharedPtr<FJsonObject>& conditionObj = conditionValue->AsObject();
                            FDialogueCondition condition;
                            condition.type = conditionObj->GetStringField(TEXT("type"));
                            condition.flag = conditionObj->GetStringField(TEXT("flag"));
                            condition.value = conditionObj->GetStringField(TEXT("value"));
                            choice.conditions.Add(condition);
                        }
                    }
                    node.choices.Add(choice);
                }
            }
            // mLoadedDialogues.Add(node.id, node);
        }
    }
    else {
        UE_LOG(LogTemp, Error, TEXT("Failed to extract dialogues from json %s"), *filePath);
        return false;
    }

    return true;
}

FString UMK_DialogueManager::findDialogueFile(const FName dialogueID) const
{
    for (const auto& keyValPair : mDialogueRegistry) {
        if (dialogueID.ToString().StartsWith(keyValPair.Key)) {
            return keyValPair.Value.filePath;
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("No file found for dialogue ID: %s"), *dialogueID.ToString());
    return FString();
}

bool UMK_DialogueManager::areChoicesConditionsMet(const TArray<FDialogueCondition>& conditions) const
{
    for (const FDialogueCondition& condition : conditions) {
        if (condition.type == "flag_set") {
            // TODO: Check game state (e.g., query a flag from game instance)
            // Placeholder: assume all conditions are true for simplicity
            return true;
        }
    }
    return true; // If no conditions, choice is available
}
