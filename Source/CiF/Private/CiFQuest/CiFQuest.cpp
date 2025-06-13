// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFQuest/CiFQuest.h"

#include "CiFCast.h"
#include "CiFItem.h"
#include "CiFManager.h"
#include "CiFCharacter.h"
#include "CiFSubsystem.h"
#include "CiFRule.h"
#include "CiFQuest/CiFQuestState.h"

bool UCiFQuest::isCompleted(const FName initiator, const FName responder)
{
    const auto cifManager = GetWorld()->GetGameInstance()->GetSubsystem<UCiFSubsystem>()->getInstance();

    const auto initiatorCharacter = static_cast<UCiFCharacter*>(cifManager->getGameObjectByName(initiator));
    const auto responderObj = cifManager->getGameObjectByName(responder);

    // If we have matched any of our completion states, then we have reached the end of the quest
    for (const auto& state : mCompletionStates) {
        if (state->mState->evaluate(initiatorCharacter, responderObj)) {
            mbCompleted = true;
            return true;
        }
    }

    // todo maybe broadcast a quest completed delegate? or do it in the caller method
    
    return false;			
}

bool UCiFQuest::checkAllPreconditions(UCiFCharacter* initiator, UCiFCast* cast, UCiFItem* item)
{
    // check giver preconditions
    for (const auto c : cast->mCharacters) {
        if (initiator != c) {
            // TODO: Check for giver preconditions by checking if there is an instantiation available for the intiator
            if (!checkPreconditions(initiator, c, item)) {
                return false;
            }
        }
    }
    return true;
}

bool UCiFQuest::checkPreconditions(UCiFCharacter* initiator, UCiFCharacter* responder, UCiFItem* item)
{
    for (const auto& p : mPreconditions) {
        if (!p->evaluate(initiator, responder, item)) {
            return false;
        }
    }
    return true;
}
