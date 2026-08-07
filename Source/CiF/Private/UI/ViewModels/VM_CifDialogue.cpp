// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/ViewModels/VM_CifDialogue.h"

#include "GLSMacroses.h"
#include "Narrative/CifInstantiation.h"

void UVM_CifDialogue::startDialogue(UCifInstantiation* instantiation)
{
    mInstantiationName = IsValid(instantiation) ? instantiation->getName() : NAME_None;
    if (!IsValid(instantiation)) {
        GLS_LOG(LogTemp, Warning, TEXT("Cannot start dialogue with an invalid instantiation"));
        finishDialogue();
        return;
    }

    mInstantiation = instantiation;
    mInstantiation->resetDialogue();
    advanceDialogue();
}

void UVM_CifDialogue::advanceDialogue()
{
    if (!IsValid(mInstantiation)) {
        finishDialogue();
        return;
    }

    FDialogueLine line;
    if (!mInstantiation->getNextDialogueLine(line)) {
        finishDialogue();
        return;
    }

    UE_MVVM_SET_PROPERTY_VALUE(mSpeaker, line.mSpeaker);
    UE_MVVM_SET_PROPERTY_VALUE(mText, line.mText);
    UE_MVVM_SET_PROPERTY_VALUE(mIsDialogueActive, true);
}

void UVM_CifDialogue::finishDialogue()
{
    mInstantiation = nullptr;

    // don't clear the speaker name and text because when clearing them before the widget
    // fades out, we see the clearing on the widget (text becomes "None")
    UE_MVVM_SET_PROPERTY_VALUE(mIsDialogueActive, false);
}
