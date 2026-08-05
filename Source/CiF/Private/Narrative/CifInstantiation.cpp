// Fill out your copyright notice in the Description page of Project Settings.


#include "Narrative/CifInstantiation.h"
#include "CiFGameObject.h"
#include "GLSMacroses.h"

namespace
{
    // Normalize a dialogue node's text so authored files may use either one string or an array of strings.
    void parseTextField(const TSharedPtr<FJsonObject>& json, const TCHAR* fieldName, TArray<FText>& outText)
    {
        TArray<FString> lines;
        if (json->TryGetStringArrayField(fieldName, lines)) {
            for (const FString& line : lines) {
                outText.Add(FText::FromString(line));
            }
            return;
        }

        FString line;
        if (json->TryGetStringField(fieldName, line)) {
            outText.Add(FText::FromString(line));
        }
    }

    FDialogueNode parseDialogueNode(const TSharedPtr<FJsonObject>& json, const int32 nodeIndex, const bool bLegacyFormat)
    {
        FDialogueNode node;

        FString nodeId;
        if (!json->TryGetStringField(TEXT("_id"), nodeId)) {
            nodeId = FString::Printf(TEXT("node_%d"), nodeIndex);
        }
        node.id = FName(nodeId);

        FString speaker;
        json->TryGetStringField(bLegacyFormat ? TEXT("_primarySpeaker") : TEXT("_speaker"), speaker);
        node.speaker = FName(speaker);

        parseTextField(json, TEXT("_text"), node.text);
        return node;
    }

    bool isDialogueNodeValid(const FDialogueNode& node, const FName instantiationName, const UObject* worldContextObj)
    {
        bool bIsValid = true;
        if (node.speaker.IsNone()) {
            GLS_LOG_CONTEXT(worldContextObj,
                            LogTemp,
                            Warning,
                            TEXT("Dialogue node %s in instantiation %s is missing its speaker"),
                            *node.id.ToString(),
                            *instantiationName.ToString());
            bIsValid = false;
        }

        if (node.text.IsEmpty()) {
            GLS_LOG_CONTEXT(worldContextObj,
                            LogTemp,
                            Warning,
                            TEXT("Dialogue node %s in instantiation %s does not contain any text"),
                            *node.id.ToString(),
                            *instantiationName.ToString());
            bIsValid = false;
        }
        return bIsValid;
    }
} // namespace

void UCifInstantiation::init(const int id, const FName name, const FText& description)
{
    mId = id;
    mName = name;
    mSrcDescription = description;
}

void UCifInstantiation::getChoicesIfAvailable(TArray<FDialogueChoice>& outChoices) const
{
    if (mDialogueNodes.IsValidIndex(mCurrentNode)) {
        outChoices = mDialogueNodes[mCurrentNode].choices;
    }
}

FText UCifInstantiation::getNextDialogueLine()
{
    const FText line = getCurrentLine();
    if (!mDialogueNodes.IsValidIndex(mCurrentNode)) {
        return line;
    }

    ++mCurrentLine;
    if (mCurrentLine >= mDialogueNodes[mCurrentNode].text.Num()) {
        ++mCurrentNode;
        mCurrentLine = 0;
    }
    return line;
}

FText UCifInstantiation::getCurrentLine() const
{
    if (mDialogueNodes.IsValidIndex(mCurrentNode) && mDialogueNodes[mCurrentNode].text.IsValidIndex(mCurrentLine)) {
        return mDialogueNodes[mCurrentNode].text[mCurrentLine];
    }
    return FText::GetEmpty();
}

FText UCifInstantiation::realizeDialogueLine(const UCiFGameObject* initiator,
                                             const UCiFGameObject* responder,
                                             const UCiFGameObject* other) const
{
    FString result = mSrcDescription.ToString();

    // replace names
    result = result.Replace(TEXT("%i%"), *initiator->mObjectName.ToString());
    if (responder) result = result.Replace(TEXT("%r%"), *responder->mObjectName.ToString());
    if (other) result = result.Replace(TEXT("%o%"), *other->mObjectName.ToString());

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
                localStartIndex -= openingDelimiter.Len();                 // include the delimiter
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
    }
    while (bFound);

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

    inst->init(instId, instName, instDesc);

    const TArray<TSharedPtr<FJsonValue>>* linesJson = nullptr;
    if (json->TryGetArrayField(TEXT("Lines"), linesJson)) {
        for (int32 index = 0; index < linesJson->Num(); ++index) {
            const TSharedPtr<FJsonObject> lineJson = (*linesJson)[index]->AsObject();
            if (lineJson.IsValid()) {
                FDialogueNode node = parseDialogueNode(lineJson, index, true);
                if (isDialogueNodeValid(node, instName, worldContextObj)) {
                    inst->mDialogueNodes.Add(MoveTemp(node));
                }
            }
        }
    }

    return inst;
}

UCifInstantiation* UCifInstantiation::loadDialogueFromJson(const TSharedPtr<FJsonObject> json, UObject* worldContextObj)
{
    FString instantiationId;
    if (!json->TryGetStringField(TEXT("_id"), instantiationId) || instantiationId.IsEmpty()) {
        GLS_LOG_CONTEXT(worldContextObj, LogTemp, Error, TEXT("Dialogue instantiation is missing a non-empty _id"));
        return nullptr;
    }

    const TArray<TSharedPtr<FJsonValue>>* dialogueJson = nullptr;
    if (!json->TryGetArrayField(TEXT("dialogue"), dialogueJson)) {
        GLS_LOG_CONTEXT(worldContextObj, LogTemp, Error, TEXT("Dialogue instantiation %s is missing its dialogue array"), *instantiationId);
        return nullptr;
    }

    UCifInstantiation* instantiation = NewObject<UCifInstantiation>(worldContextObj);
    instantiation->init(CIF_INVALID_ID, FName(instantiationId), FText::GetEmpty());
    for (int32 index = 0; index < dialogueJson->Num(); ++index) {
        const TSharedPtr<FJsonObject> nodeJson = (*dialogueJson)[index]->AsObject();
        if (!nodeJson.IsValid()) {
            GLS_LOG_CONTEXT(
                worldContextObj, LogTemp, Warning, TEXT("Ignoring invalid dialogue node %d in instantiation %s"), index, *instantiationId);
            continue;
        }

        FDialogueNode node = parseDialogueNode(nodeJson, index, false);
        if (isDialogueNodeValid(node, FName(instantiationId), worldContextObj)) {
            instantiation->mDialogueNodes.Add(MoveTemp(node));
        }
    }

    return instantiation;
}
