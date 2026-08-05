// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CifNarrativeManager.generated.h"

class UMK_DialogueManager;
class UCifInstantiation;
class UCifPlotPointPool;
struct FCifPlotPointSelection;

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
     * Checks whether a character, item, or other named game object can reveal an available plot point.
     * @param revealer named
     * game object attempting to reveal a plot point
     * @param outSelection receives both the matching plot point and its specific
     * revelation route
     * @return true when a matching revelation was found
     */
    bool findPlotPointToPlay(const FName revealer, FCifPlotPointSelection& outSelection) const;

    void getInstantiationForSocialGame(const FName sgName, const FName initiator, const FName responder, const FName other = NAME_None);

    /** Resolves the selected revelation route into its registered dialogue instantiation. */
    UCifInstantiation* getInstantiationForPlotPoint(const FCifPlotPointSelection& selection);

private:
    void loadPlotPoints(const FString& filePath, const UObject* worldContextObject);

    UPROPERTY()
    UCifPlotPointPool* mPlotPointPool;

    UPROPERTY()
    UMK_DialogueManager* mDialogueMgr;
};
