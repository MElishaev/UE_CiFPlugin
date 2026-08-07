// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/MKUI_W_ActivatableBase.h"
#include "W_CifDialogue.generated.h"

class UCifInstantiation;
class UCommonButtonBase;
class UVM_CifDialogue;

/**
 * Common UI screen that presents one CiF dialogue through its native ViewModel.
 *
 * Widget Blueprint contract:
 * - Add UVM_CifDialogue to the MVVM panel with Manual creation and bind its speaker/text fields.
 * - Provide a UCommonButtonBase named mAdvanceButton that covers the screen and remains hit-testable.
 * - Place visual content above the button, but make that content hit-test-invisible so clicks reach the button.
 *
 * The focused button receives mouse clicks directly. Slate's default navigation maps Space and Enter to the
 * UI Accept action, which also clicks the focused button; no separate Enhanced Input mapping is required.
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

protected:
    virtual void NativeOnInitialized() override;

    /** Focuses the full-screen button so the standard UI Accept action advances the dialogue. */
    virtual UWidget* NativeGetDesiredFocusTarget() const override;

    /** Blocks lower-priority gameplay input while the dialogue is the active Common UI screen. */
    virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;

private:
    /** Required WBP button. Keep it Visible with zero render opacity; do not make it hit-test-invisible. */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> mAdvanceButton;

    UPROPERTY(Transient)
    TObjectPtr<UVM_CifDialogue> mDialogueViewModel;
};
