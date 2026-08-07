// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widgets/W_CifDialogue.h"

#include "CommonButtonBase.h"
#include "CommonInputModeTypes.h"
#include "GLSMacroses.h"
#include "InputCoreTypes.h"
#include "Input/UIActionBindingHandle.h"
#include "MVVMSubsystem.h"
#include "Narrative/CifInstantiation.h"
#include "UI/ViewModels/VM_CifDialogue.h"
#include "View/MVVMView.h"

void UW_CifDialogue::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    checkf(mAdvanceButton, TEXT("Dialogue widget %s requires a bound Common Button named mAdvanceButton"), *GetName());
    mAdvanceButton->OnClicked().AddUObject(this, &ThisClass::advanceDialogue);
}

UWidget* UW_CifDialogue::NativeGetDesiredFocusTarget() const
{
    return mAdvanceButton ? mAdvanceButton.Get() : Super::NativeGetDesiredFocusTarget();
}

TOptional<FUIInputConfig> UW_CifDialogue::GetDesiredInputConfig() const
{
    return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture, false);
}

FReply UW_CifDialogue::NativeOnPreviewKeyDown(const FGeometry& inGeometry, const FKeyEvent& inKeyEvent)
{
    const FKey pressedKey = inKeyEvent.GetKey();
    if (pressedKey == EKeys::SpaceBar || pressedKey == EKeys::Enter) {
        // Consume repeats as well, but advance only once per physical key press.
        if (!inKeyEvent.IsRepeat()) {
            advanceDialogue();
        }
        return FReply::Handled();
    }

    return Super::NativeOnPreviewKeyDown(inGeometry, inKeyEvent);
}

bool UW_CifDialogue::initDialogue(UCifInstantiation* instantiation)
{
    if (!IsValid(instantiation)) {
        GLS_LOG(LogTemp, Warning, TEXT("Cannot initialize dialogue widget with an invalid instantiation"));
        return false;
    }

    UMVVMView* view = UMVVMSubsystem::GetViewFromUserWidget(this);
    if (!view) {
        GLS_LOG(LogTemp,
                Error,
                TEXT("Dialogue widget %s has no MVVM view. Add UVM_CifDialogue as a Manual ViewModel source in its Widget Blueprint"),
                *GetName());
        return false;
    }

    if (!mDialogueViewModel) {
        mDialogueViewModel = NewObject<UVM_CifDialogue>(this);
    }

    TScriptInterface<INotifyFieldValueChanged> viewModelInterface;
    viewModelInterface.SetObject(mDialogueViewModel);
    viewModelInterface.SetInterface(mDialogueViewModel);
    if (!view->SetViewModelByClass(viewModelInterface)) {
        GLS_LOG(LogTemp,
                Error,
                TEXT("Dialogue widget %s could not set UVM_CifDialogue. Ensure its ViewModel creation type is Manual"),
                *GetName());
        return false;
    }

    mDialogueViewModel->startDialogue(instantiation);
    return mDialogueViewModel->isDialogueActive();
}

void UW_CifDialogue::advanceDialogue()
{
    if (!mDialogueViewModel) {
        GLS_LOG(LogTemp, Warning, TEXT("Cannot advance uninitialized dialogue widget %s"), *GetName());
        return;
    }

    mDialogueViewModel->advanceDialogue();
    if (!mDialogueViewModel->isDialogueActive()) {
        OnDialogueFinished.Broadcast(mDialogueViewModel->getInstantiationName());
        DeactivateWidget();
    }
}
