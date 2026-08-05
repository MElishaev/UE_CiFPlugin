// MAAKU Studio all rights reserved


#include "Narrative/MK_DialogueManager.h"

#include "GLSMacroses.h"
#include "Narrative/CifInstantiation.h"
#include "ReadWriteFiles.h"

bool UMK_DialogueManager::initializeRegistry(const FString& registryPath)
{
    TSharedPtr<FJsonObject> jsonObject;
    if (!UReadWriteFiles::readJson(registryPath, jsonObject)) {
        GLS_LOG(LogTemp, Error, TEXT("Failed to read json %s"), *registryPath);
        return false;
    }

    mDialogueRegistry.Empty();
    mRegistryDirectory = FPaths::GetPath(registryPath);
    const TArray<TSharedPtr<FJsonValue>>* filesArray;
    if (jsonObject->TryGetArrayField(TEXT("instantiations"), filesArray)) {
        for (const TSharedPtr<FJsonValue>& fileElem : *filesArray) {
            const TSharedPtr<FJsonObject>& fileObj = fileElem->AsObject();
            if (!fileObj.IsValid()) {
                GLS_LOG(LogTemp, Warning, TEXT("Ignoring invalid entry in dialogue registry %s"), *registryPath);
                continue;
            }

            FString instantiationId;
            FString filePath;
            if (!fileObj->TryGetStringField(TEXT("id"), instantiationId) || instantiationId.IsEmpty() ||
                !fileObj->TryGetStringField(TEXT("file_path"), filePath) || filePath.IsEmpty()) {
                GLS_LOG(LogTemp, Warning, TEXT("Ignoring dialogue registry entry without a valid id and file_path"));
                continue;
            }

            FDialogueFileEntry entry;
            entry.mInstantiationId = FName(instantiationId);
            entry.mFilePath = MoveTemp(filePath);
            mDialogueRegistry.Add(entry.mInstantiationId, entry);
        }
        return true;
    }

    GLS_LOG(LogTemp, Error, TEXT("Dialogue registry %s is missing its instantiations array"), *registryPath);
    return false;
}

UCifInstantiation* UMK_DialogueManager::prepareDialogue(const FName instantiationId)
{
    if (UCifInstantiation* instantiation = mLoadedDialogues.FindRef(instantiationId)) {
        instantiation->resetDialogue();
        mCurrentDialogueID = instantiationId;
        return instantiation;
    }

    const FString filePath = findDialogueFile(instantiationId);
    if (filePath.IsEmpty()) {
        return nullptr;
    }

    if (!loadDialogueFile(filePath)) {
        return nullptr;
    }

    UCifInstantiation* instantiation = mLoadedDialogues.FindRef(instantiationId);
    if (!instantiation) {
        GLS_LOG(LogTemp,
                Error,
                TEXT("Dialogue file %s did not contain its registered instantiation %s"),
                *filePath,
                *instantiationId.ToString());
        return nullptr;
    }

    instantiation->resetDialogue();
    mCurrentDialogueID = instantiationId;
    return instantiation;
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
        GLS_LOG(LogTemp, Error, TEXT("Failed to read json %s"), *filePath);
        return false;
    }

    UCifInstantiation* instantiation = UCifInstantiation::loadDialogueFromJson(jsonObject, this);
    if (!instantiation) {
        GLS_LOG(LogTemp, Error, TEXT("Failed to load dialogue instantiation from json %s"), *filePath);
        return false;
    }

    mLoadedDialogues.Add(instantiation->getName(), instantiation);
    return true;
}

FString UMK_DialogueManager::findDialogueFile(const FName instantiationId) const
{
    const FDialogueFileEntry* entry = mDialogueRegistry.Find(instantiationId);
    if (!entry) {
        GLS_LOG(LogTemp, Warning, TEXT("No file found for instantiation ID: %s"), *instantiationId.ToString());
        return FString();
    }

    return FPaths::IsRelative(entry->mFilePath) ? FPaths::Combine(mRegistryDirectory, entry->mFilePath) : entry->mFilePath;
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
