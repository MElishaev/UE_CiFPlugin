// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Presenters/CifDialoguePresenter.h"

#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "GLSMacroses.h"
#include "GameFramework/PlayerController.h"
#include "MKUI_GameplayTags.h"
#include "Narrative/CifInstantiation.h"
#include "Subsystems/MKUI_Subsystem.h"
#include "UI/Widgets/W_CifDialogue.h"
#include "Widgets/MKUI_W_ActivatableBase.h"

void UCifDialoguePresenter::Deinitialize()
{
    if (mActiveDialogueWidget) {
        mActiveDialogueWidget->OnDialogueFinished.RemoveDynamic(this, &ThisClass::handleDialogueFinished);
        mActiveDialogueWidget->OnDeactivated().RemoveAll(this);
    }

    mActiveDialogueWidget = nullptr;
    mPendingInstantiation = nullptr;
    Super::Deinitialize();
}

bool UCifDialoguePresenter::presentDialogue(UCifInstantiation* instantiation, TSoftClassPtr<UW_CifDialogue> dialogueWidgetClass)
{
    if (!IsValid(instantiation)) {
        GLS_LOG(LogTemp, Warning, TEXT("Cannot present an invalid dialogue instantiation"));
        return false;
    }
    if (dialogueWidgetClass.IsNull()) {
        GLS_LOG(LogTemp, Warning, TEXT("Cannot present dialogue %s without a dialogue widget class"), *instantiation->getName().ToString());
        return false;
    }
    if (mPendingInstantiation || mActiveDialogueWidget) {
        GLS_LOG(LogTemp, Warning, TEXT("Cannot present dialogue %s while another dialogue is pending or active"), *instantiation->getName().ToString());
        return false;
    }

    ULocalPlayer* localPlayer = GetLocalPlayer();
    UGameInstance* gameInstance = localPlayer ? localPlayer->GetGameInstance() : nullptr;
    UMKUI_Subsystem* uiSubsystem = gameInstance ? gameInstance->GetSubsystem<UMKUI_Subsystem>() : nullptr;
    APlayerController* playerController = localPlayer ? localPlayer->GetPlayerController(GetWorld()) : nullptr;
    if (!uiSubsystem || !playerController) {
        GLS_LOG(LogTemp, Error, TEXT("Cannot present dialogue because the local player's MK_UI subsystem or PlayerController is unavailable"));
        return false;
    }

    mPendingInstantiation = instantiation;
    mbPendingInitializationSucceeded = false;

    const TWeakObjectPtr<ThisClass> weakThis(this);
    const TSoftClassPtr<UMKUI_W_ActivatableBase> activatableWidgetClass(dialogueWidgetClass.ToSoftObjectPath());
    uiSubsystem->pushSoftWidgetToStackAsync(
        MKUI_GameplayTags::MKUI_widgetStack_modal,
        activatableWidgetClass,
        [weakThis](const EAsyncPushWidgetState pushState, UMKUI_W_ActivatableBase* pushedWidget) {
            if (ThisClass* presenter = weakThis.Get()) {
                presenter->handleWidgetPush(pushState, pushedWidget);
            }
        });
    return true;
}

bool UCifDialoguePresenter::isPresentingDialogue() const
{
    return mPendingInstantiation || mActiveDialogueWidget;
}

void UCifDialoguePresenter::handleWidgetPush(const EAsyncPushWidgetState pushState, UMKUI_W_ActivatableBase* pushedWidget)
{
    if (pushState == EAsyncPushWidgetState::OnCreatedBeforePush) {
        mActiveDialogueWidget = Cast<UW_CifDialogue>(pushedWidget);
        if (!mActiveDialogueWidget) {
            GLS_LOG(LogTemp, Error, TEXT("Configured dialogue widget must inherit UW_CifDialogue"));
            return;
        }

        mActiveDialogueWidget->SetOwningPlayer(GetLocalPlayer()->GetPlayerController(GetWorld()));
        mActiveDialogueWidget->OnDialogueFinished.AddUniqueDynamic(this, &ThisClass::handleDialogueFinished);
        mActiveDialogueWidget->OnDeactivated().AddUObject(this, &ThisClass::handleDialogueDeactivated);
        mbPendingInitializationSucceeded = mActiveDialogueWidget->initDialogue(mPendingInstantiation);
        return;
    }

    if (pushState == EAsyncPushWidgetState::AfterPush) {
        mPendingInstantiation = nullptr;
        if (!mbPendingInitializationSucceeded && pushedWidget) {
            pushedWidget->DeactivateWidget();
        }
    }
}

void UCifDialoguePresenter::handleDialogueFinished(const FName instantiationName)
{
    OnDialogueFinished.Broadcast(instantiationName);
}

void UCifDialoguePresenter::handleDialogueDeactivated()
{
    if (mActiveDialogueWidget) {
        mActiveDialogueWidget->OnDialogueFinished.RemoveDynamic(this, &ThisClass::handleDialogueFinished);
        mActiveDialogueWidget->OnDeactivated().RemoveAll(this);
        mActiveDialogueWidget = nullptr;
    }
}
