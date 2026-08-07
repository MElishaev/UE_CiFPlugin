// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/MKUI_W_ActivatableBase.h"
#include "W_CifDialogue.generated.h"

class UCifInstantiation;
class UCommonButtonBase;
class UVM_CifDialogue;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueFinished, FName, instantiationName);

/**
 * Common UI screen that presents one CiF dialogue through its native ViewModel.
 *
 * Widget Blueprint contract:
 * - Add UVM_CifDialogue to the MVVM panel with Manual creation and bind its speaker/text fields.
 * - Provide a UCommonButtonBase named mAdvanceButton that covers the screen and remains hit-testable.
 * - Place visual content above the button, but make that content hit-test-invisible so clicks reach the button.
 *
 * The focused button receives mouse clicks directly. This widget explicitly handles Space and Enter so the
 * dialogue controls do not depend on which keys the host project's Common UI Accept action maps.
 */
UCLASS(Abstract, BlueprintType, meta = (DisableNativeTick))
class CIF_API UW_CifDialogue : public UMKUI_W_ActivatableBase
{
    GENERATED_BODY()

public:
    /**
     * Installs the native ViewModel into this Widget Blueprint's manual MVVM source and starts playback.
     * Call during MK_UI's On Created Before Push callback so the first line is ready when the screen activates.
     */
    UFUNCTION(BlueprintCallable, Category = "CiF|UI|Dialogue")
    bool initDialogue(UCifInstantiation* instantiation);

    /** Advances playback and deactivates this screen after the final displayed line. */
    UFUNCTION(BlueprintCallable, Category = "CiF|UI|Dialogue")
    void advanceDialogue();

    UFUNCTION(BlueprintPure, Category = "CiF|UI|Dialogue")
    UVM_CifDialogue* getDialogueViewModel() const { return mDialogueViewModel; }

    UPROPERTY(BlueprintAssignable, Category = "CiF|UI|Dialogue")
    FOnDialogueFinished OnDialogueFinished;

protected:
    virtual void NativeOnInitialized() override;

    /** Focuses the full-screen button so mouse and configured Common UI Accept input still work naturally. */
    virtual UWidget* NativeGetDesiredFocusTarget() const override;

    /** Blocks lower-priority gameplay input while the dialogue is the active Common UI screen. */
    virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;

    /** Advances on Space or Enter and consumes those keys before the focused child or gameplay receives them. */
    virtual FReply NativeOnPreviewKeyDown(const FGeometry& inGeometry, const FKeyEvent& inKeyEvent) override;

private:
    /** Required WBP button. Keep it Visible with zero render opacity; do not make it hit-test-invisible. */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> mAdvanceButton;

    UPROPERTY(Transient)
    TObjectPtr<UVM_CifDialogue> mDialogueViewModel;
};
