// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "VM_CifDialogue.generated.h"

class UCifInstantiation;

/** Exposes an active CiF dialogue as bindable UI state. */
UCLASS(BlueprintType, Blueprintable, DisplayName = "CiF Dialogue ViewModel")
class CIF_API UVM_CifDialogue : public UMVVMViewModelBase
{
    GENERATED_BODY()

public:
    /** Rewinds the supplied instantiation and immediately presents its first line. */
    UFUNCTION(BlueprintCallable, Category = "CiF|UI|Dialogue")
    void startDialogue(UCifInstantiation* instantiation);

    /** Presents the next line, or ends the dialogue when no lines remain. */
    UFUNCTION(BlueprintCallable, Category = "CiF|UI|Dialogue")
    void advanceDialogue();

    UFUNCTION(BlueprintPure, Category = "CiF|UI|Dialogue")
    bool isDialogueActive() const { return mIsDialogueActive; }

private:
    void finishDialogue();

    UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "CiF|UI|Dialogue", meta = (AllowPrivateAccess = "true"))
    FName mSpeaker = NAME_None;

    UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "CiF|UI|Dialogue", meta = (AllowPrivateAccess = "true"))
    FText mText;

    UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "CiF|UI|Dialogue", meta = (AllowPrivateAccess = "true"))
    bool mIsDialogueActive = false;

    UPROPERTY(Transient)
    TObjectPtr<UCifInstantiation> mInstantiation;
};
