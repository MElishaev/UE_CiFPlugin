// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CifNarrativeManager.generated.h"

class UMK_DialogueManager;
class UCifPlotPointPool;

/**
 * This class should manage the narrative of the game.
 * It will track what the player already did and what is available, it will decide when
 * and if to reveal narrative bits etc.
 */
UCLASS()
class CIF_API UCifNarrativeManager : public UObject
{
    GENERATED_BODY()

public:

    void init();

    /**
     * While in a social game that is playing out, check if the responder in the social game has a plot point
     * to reveal to the player. Return the plot point ID to be played
     * @param responder the responder in the current playing SG
     * @return plot point name or NONE if no plot point to play
     * todo but why would we want the pp name to be returned? y not directly start playing the pp?
     * just do get dialogue if pp available to be played and thats it.
     */
    FName getPPNameToBePlayed(const FName responder) const;

    void getInstantiationForSocialGame(const FName sgName, const FName initiator, const FName responder, const FName other=NAME_None);

    // where participant is the character which is not player (could be responder or initiator if npc started interaction)
    void getInstantiationForPlotPoint(const FName ppName, const FName participant);
private:

    void loadPlotPoints(const FString& filePath, const UObject* worldContextObject);

    UPROPERTY()
    UCifPlotPointPool* mPlotPointPool;

    UPROPERTY()
    UMK_DialogueManager* mDialogueMgr;
};
