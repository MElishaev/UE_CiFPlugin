// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "CifDialoguePresenter.generated.h"

class APlayerController;
class UCifInstantiation;
class UMKUI_W_ActivatableBase;
class UW_CifDialogue;
enum class EAsyncPushWidgetState : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCifPresentedDialogueFinished, FName, instantiationName);

/**
 * Presents CiF dialogue for one local player through MK_UI.
 *
 * The presenter owns the widget push, initialization, and event-binding lifecycle. It forwards completion to
 * the requesting game layer without deciding what completing a particular instantiation means.
 */
UCLASS()
class CIF_API UCifDialoguePresenter : public ULocalPlayerSubsystem
{
    GENERATED_BODY()

public:
    virtual void Deinitialize() override;

    /**
     * Asynchronously pushes and initializes a dialogue widget on MK_UI's modal stack.
     * @return true when the presentation request was accepted
     */
    UFUNCTION(BlueprintCallable, Category = "CiF|UI|Dialogue")
    bool presentDialogue(UCifInstantiation* instantiation, TSoftClassPtr<UW_CifDialogue> dialogueWidgetClass);

    UFUNCTION(BlueprintPure, Category = "CiF|UI|Dialogue")
    bool isPresentingDialogue() const;

    /** Forwarded only when the player naturally advances beyond the dialogue's final line. */
    UPROPERTY(BlueprintAssignable, Category = "CiF|UI|Dialogue")
    FOnCifPresentedDialogueFinished OnDialogueFinished;

private:
    void handleWidgetPush(EAsyncPushWidgetState pushState, UMKUI_W_ActivatableBase* pushedWidget);
    void handleDialogueDeactivated();

    UFUNCTION()
    void handleDialogueFinished(FName instantiationName);

    UPROPERTY(Transient)
    TObjectPtr<UCifInstantiation> mPendingInstantiation;

    UPROPERTY(Transient)
    TObjectPtr<UW_CifDialogue> mActiveDialogueWidget;

    bool mbPendingInitializationSucceeded = false;
};
