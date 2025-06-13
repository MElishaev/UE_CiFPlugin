// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CiFQuestState.generated.h"

class UCiFRule;

/**
 * This class describes a starting or a completion state of a quest.
 */
UCLASS()
class CIF_API UCiFQuestState : public UObject
{
	GENERATED_BODY()

public:

    UPROPERTY()
    FName mName;

    /* the social state that acts as a pre-requisite for starting or completing the quest, based on
     * if this class instance represents a starting and completion state.
     */
    UPROPERTY()
    UCiFRule* mState; 

    /* associated scenes to this quest state. for example, if this state represents a starting state,
     * then this will hold all the different scenes that can be used to start this quest.
     * generally, for starting states there will be 1 scene, because each starting state will have corresponding
     * dialogue to start the quest, but for completion state, this will hold usually more than 1 scene to
     * represent every possible way the player can complete the quest while matched to the starting state, which
     * the player could choose to complete the quest differently from what he was asked.
     */ 
    // UPROPERTY()
    // TArray<UCiFScene*> mScenes; 
};
