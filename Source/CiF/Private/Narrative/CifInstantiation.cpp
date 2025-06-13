// Fill out your copyright notice in the Description page of Project Settings.


#include "Narrative/CifInstantiation.h"

void UCifInstantiation::assignDialogueTree(TSoftObjectPtr<UDataTable> dtRef)
{
    mDialogueTable = dtRef.Get();
    if (!mDialogueTable) {
        // Load synchronously if not already loaded todo - change to be loaded only when needed and not always
        mDialogueTable = dtRef.LoadSynchronous();
        if (mDialogueTable) {
            UE_LOG(LogTemp, Log, TEXT("Assigned DataTable: %s"), *dtRef.ToString());
        }
        else {
            UE_LOG(LogTemp, Warning, TEXT("Failed to load DataTable: %s"), *dtRef.ToString());
        }
    }
}

UCifInstantiation* UCifInstantiation::loadFromJson(const TSharedPtr<FJsonObject> json, UObject* worldContextObj)
{
    const auto inst = NewObject<UCifInstantiation>(worldContextObj);

    auto text = FText::FromString(json->GetStringField(TEXT("_text")));
    
    
    return inst;
}
