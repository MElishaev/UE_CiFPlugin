// Fill out your copyright notice in the Description page of Project Settings.


#include "Narrative/CifInstantiation.h"
#include "CiFGameObject.h"

void UCifInstantiation::init(const int id, const FName name, const FText& description)
{
    mId = id;
    mName = name;
    mSrcDescription = description;
}

void UCifInstantiation::getChoicesIfAvailable(TArray<FDialogueChoice>& outChoices) const
{
    // todo - implement
    return;
}

FText UCifInstantiation::getNextDialogueLine()
{
    auto line = getCurrentLine();
    mCurrentLine++;
    
    // if (mCurrentLine >= mLoadedDialogues[mCurrentDialogueID].text.Num()) {
    //     mCurrentLine = 0;
    //     // todo: maybe now clear this dialogue from memory? if the LoadedDialogue is full or something?
    // }S

    return line;
}

FText UCifInstantiation::getCurrentLine() const
{
    // if (mLoadedDialogues.Contains(mCurrentDialogueID)) {
    //     auto lines = mLoadedDialogues[mCurrentDialogueID].text;
    //     return lines[mCurrentLine];
    // }
    return FText::GetEmpty();
}

FText UCifInstantiation::realizeDialogueLine(const UCiFGameObject* initiator, const UCiFGameObject* responder, const UCiFGameObject* other) const
{
    FString result = mSrcDescription.ToString();

    // replace names
    result = result.Replace(TEXT("%i%"), *initiator->mObjectName.ToString());
    if (responder) result = result.Replace(TEXT("%r%"), *responder->mObjectName.ToString());
    if (other) result = result.Replace(TEXT("%o%"), *other->mObjectName.ToString());

    // replace subjective (?) words
    FString subj = "her";
    if (initiator->hasTrait(ETrait::MALE)) {
        subj = "him";
    }
    result = result.Replace(TEXT("%gender(him, her)%"), *subj);

    return FText::FromString(result);
}

FText UCifInstantiation::getmDescription(const UCiFGameObject* initiator, const UCiFGameObject* responder, const UCiFGameObject* other) const
{
    return realizeDialogueLine(initiator, responder, other);
}

bool UCifInstantiation::requiresOtherToPerform() const
{
    // todo - implement?
    return false;
}

UCifInstantiation* UCifInstantiation::loadFromJson(const TSharedPtr<FJsonObject> json, UObject* worldContextObj)
{
    const auto inst = NewObject<UCifInstantiation>(worldContextObj);

    const ID_t instId = json->GetIntegerField(TEXT("_id"));
    const FName instName = FName(json->GetStringField(TEXT("_name")));
    const FText instDesc = FText::FromString(json->GetStringField(TEXT("_description"))); 

    inst->init(instId, instName, instDesc);
    return inst;
}
