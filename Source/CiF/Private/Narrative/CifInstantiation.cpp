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

FText UCifInstantiation::realizeDialogueLine(const UCiFGameObject* initiator,
                                             const UCiFGameObject* responder,
                                             const UCiFGameObject* other) const
{
    FString result = mSrcDescription.ToString();

    // replace names
    result = result.Replace(TEXT("%i%"), *initiator->mObjectName.ToString());
    if (responder)
        result = result.Replace(TEXT("%r%"), *responder->mObjectName.ToString());
    if (other)
        result = result.Replace(TEXT("%o%"), *other->mObjectName.ToString());

    result = replaceInlineDialogueOperators(result, initiator, responder, other);

    return FText::FromString(result);
}

FString UCifInstantiation::extractSubstringBetweenDelimiter(const FString& srcStr,
                                                            const FString& openingDelimiter,
                                                            const FString& closingDelimiter,
                                                            int32 startIndex,
                                                            const bool includingDelimiter) const
{
    FString result = TEXT("");

    int32 localStartIndex = srcStr.Find(openingDelimiter, ESearchCase::CaseSensitive, ESearchDir::FromStart, startIndex);
    if (localStartIndex != INDEX_NONE) {
        localStartIndex += openingDelimiter.Len();
        int32 endIndex = srcStr.Find(closingDelimiter, ESearchCase::CaseSensitive, ESearchDir::FromStart, localStartIndex);

        if (endIndex != INDEX_NONE) {
            int32 length = endIndex - localStartIndex;
            if (includingDelimiter) {
                length += openingDelimiter.Len() + closingDelimiter.Len(); // from both sides
                localStartIndex -= openingDelimiter.Len();                // include the delimiter
            }
            result = srcStr.Mid(localStartIndex, length);
        }
    }

    return result;
}

FString UCifInstantiation::replaceInlineDialogueOperators(const FString& inStr,
                                                          const UCiFGameObject* initiator,
                                                          const UCiFGameObject* responder,
                                                          const UCiFGameObject* other) const
{
    // replace operators like %gender(i,his,her)% -- %gender(o,prick,bitch)% -- %gender(r,he,she)% --
    // %random(touch me behind the ear like that,kiss my ear lobe,kiss my neck so tenderly,breathe on my neck just so)% --
    // %sweetie% -- %pejorative% which are located in the cast.json
    FString result = inStr;
    bool bFound = true;
    do {
        FString strToReplace = extractSubstringBetweenDelimiter(result, TEXT("%"), TEXT("%"));
        bFound = !strToReplace.IsEmpty();
        if (!bFound) break;

        if (strToReplace.StartsWith(TEXT("%gender"))) {
            FString strVars = extractSubstringBetweenDelimiter(strToReplace, TEXT("("), TEXT(")"), 0, false);
            TArray<FString> varsArray;
            strVars.ParseIntoArray(varsArray, TEXT("|"));
            if (varsArray[0].Compare(TEXT("i")) == 0) {
                if (initiator && initiator->hasTrait(ETrait::MALE)) {
                    result = result.Replace(*strToReplace, *varsArray[1]);
                }
                else {
                    result = result.Replace(*strToReplace, *varsArray[2]);
                }
            }
            else if (varsArray[0].Compare(TEXT("r")) == 0) {
                if (responder && responder->hasTrait(ETrait::MALE)) {
                    result = result.Replace(*strToReplace, *varsArray[1]);
                }
                else {
                    result = result.Replace(*strToReplace, *varsArray[2]);
                }
            }
            else {
                if (other && other->hasTrait(ETrait::MALE)) {
                    result = result.Replace(*strToReplace, *varsArray[1]);
                }
                else {
                    result = result.Replace(*strToReplace, *varsArray[2]);
                }
            }
        }
        else if (strToReplace.StartsWith(TEXT("%random"))) {
            FString strVars = extractSubstringBetweenDelimiter(strToReplace, TEXT("("), TEXT(")"), 0, false);
            TArray<FString> varsArray;
            strVars.ParseIntoArray(varsArray, TEXT("|"));
            if (!varsArray.IsEmpty()) {
                int32 randomOption = FMath::RandRange(0, varsArray.Num() - 1);
                result = result.Replace(*strToReplace, *varsArray[randomOption]);
            }
        }
        else {
            // todo: for other stuff that aren't implemented yet like %sweetie% etc.
        }
    } while (bFound); 

    return result;
}

FText UCifInstantiation::getmDescription(const UCiFGameObject* initiator,
                                         const UCiFGameObject* responder,
                                         const UCiFGameObject* other) const
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

    // todo: now the instantiation has also lines that can be parsed. some instantiations are actually dialogue,
    //  but the other are just short description of the interaction. need to distinguish between them

    inst->init(instId, instName, instDesc);
    return inst;
}
