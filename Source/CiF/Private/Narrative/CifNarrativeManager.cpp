// Fill out your copyright notice in the Description page of Project Settings.


#include "Narrative/CifNarrativeManager.h"
#include "Narrative/CifPlotPointPool.h"
#include "ReadWriteFiles.h"

void UCifNarrativeManager::init()
{
    const FString plotpointsPath = FPaths::Combine(*FPaths::ProjectPluginsDir(), *FString("CiF/Content/Data/plotpoints.json"));
    UE_LOG(LogTemp, Log, TEXT("Reading plot points from %s"), *plotpointsPath);
    loadPlotPoints(plotpointsPath, this);

    initializeRegistry(FPaths::Combine(*FPaths::ProjectPluginsDir(), *FString("CiF/Content/Data/Dialogue/dialogue_registry.json")));
}

void UCifNarrativeManager::loadPlotPoints(const FString& filePath, const UObject* worldContextObject)
{
    TSharedPtr<FJsonObject> jsonObject;
    if (!UReadWriteFiles::readJson(filePath, jsonObject)) {
        UE_LOG(LogTemp, Error, TEXT("Failed to read json %s"), *filePath);
        return;
    }

    const auto ppJson = jsonObject->GetArrayField(TEXT("Plotpoints"));
    mPlotPointPool = UCifPlotPointPool::loadFromJson(ppJson, worldContextObject);
    if (!mPlotPointPool) {
        UE_LOG(LogTemp, Error, TEXT("Failed to create plot point pool"));
    }
}

bool UCifNarrativeManager::initializeRegistry(const FString& registryPath)
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

FName UCifNarrativeManager::getPPNameToBePlayed(const FName responder) const
{
    return mPlotPointPool->getAvailablePPByResponder(responder);
}

void UCifNarrativeManager::prepareDialogue(const FName& dialogueID)
{
    // Check if dialogue is already loaded
    if (mLoadedDialogues.Contains(dialogueID)) {
        mCurrentDialogueID = dialogueID;
        return;
    }

    // Find and load the relevant file
    const FString filePath = findDialogueFile(dialogueID);
    if (!filePath.IsEmpty()) {
        if (loadDialogueFile(filePath)) {
            if (mLoadedDialogues.Contains(dialogueID)) {
                mCurrentDialogueID = dialogueID;
            }
        }
    }
}

FString UCifNarrativeManager::findDialogueFile(const FName dialogueID) const
{
    for (const auto& entry : mDialogueRegistry) {
        if (dialogueID.ToString().StartsWith(entry.Key)) {
            return entry.Value.filePath;
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("No file found for dialogue ID: %s"), *dialogueID.ToString());
    return FString();
}

bool UCifNarrativeManager::loadDialogueFile(const FString& filePath)
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

                    // parse conditions
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
            mLoadedDialogues.Add(node.id, node);
        }
    }
    else {
        UE_LOG(LogTemp, Error, TEXT("Failed to extract dialogues from json %s"), *filePath);
        return false;
    }
    
    return true;
}

FText UCifNarrativeManager::getCurrentText() const
{
    if (mLoadedDialogues.Contains(mCurrentDialogueID)) {
        auto lines = mLoadedDialogues[mCurrentDialogueID].text;
        return lines[mCurrentDialogueLineNum]; // todo: this method doesn't increment the dialogue line number so make sure the caller does that
    }
    return FText::GetEmpty();
}

TArray<FDialogueChoice> UCifNarrativeManager::getChoices() const
{
    TArray<FDialogueChoice> availableChoices;
    if (mLoadedDialogues.Contains(mCurrentDialogueID)) {
        for (const FDialogueChoice& choice : mLoadedDialogues[mCurrentDialogueID].choices) {
            if (areChoicesConditionsMet(choice.conditions)) {
                availableChoices.Add(choice);
            }
        }
    }
    return availableChoices;
}

void UCifNarrativeManager::selectChoice(int32 choiceIndex)
{
    if (mLoadedDialogues.Contains(mCurrentDialogueID)) {
        const TArray<FDialogueChoice>& choices = mLoadedDialogues[mCurrentDialogueID].choices;
        if (choiceIndex >= 0 && choiceIndex < choices.Num() && areChoicesConditionsMet(choices[choiceIndex].conditions)) {
            mCurrentDialogueID = choices[choiceIndex].nextID;
        }
    }
}

void UCifNarrativeManager::clearDialogues(const FString& prefix) {
    for (auto it = mLoadedDialogues.CreateIterator(); it; ++it) {
        if (it.Key().ToString().StartsWith(prefix)) {
            it.RemoveCurrent();
        }
    }
}

FText UCifNarrativeManager::getNextDialogueLine()
{
    auto line = getCurrentText();
    mCurrentDialogueLineNum++;

    if (mCurrentDialogueLineNum >= mLoadedDialogues[mCurrentDialogueID].text.Num()) {
        mCurrentDialogueLineNum = 0;
        // todo: maybe now clear this dialogue from memory? if the LoadedDialogue is full or something?
    }

    return line;
}


bool UCifNarrativeManager::areChoicesConditionsMet(const TArray<FDialogueCondition>& conditions) const
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
