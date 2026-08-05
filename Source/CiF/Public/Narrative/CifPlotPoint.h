// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CiFKnowledge.h"
#include "CoreMinimal.h"
#include "CifPlotPoint.generated.h"

class UCiFRule;

USTRUCT()
struct CIF_API FCifPlotPointRevelation
{
    GENERATED_BODY()

    // Characters, items, or other named game objects that can use this revelation route.
    UPROPERTY(VisibleAnywhere, Category = "CiF|Narrative")
    TArray<FName> mRevealedBy;

    // Identifies the complete dialogue performance used when this route reveals the plot point.
    UPROPERTY(VisibleAnywhere, Category = "CiF|Narrative")
    FName mInstantiationId = NAME_None;
};

/**
 * This class is a UCifGameObject and it represents a plot point in the narrative if any.
 * The plot points are initialized at the beginning of game and never shouldn't be deleted.
 * PlotPointPool aggregates all the possible plot points in the game.
 */
UCLASS()
class CIF_API UCifPlotPoint : public UCiFKnowledge
{
    GENERATED_BODY()

public:
    static UCifPlotPoint* loadFromJson(const TSharedPtr<FJsonObject> json, const UObject* worldContextObject);

    void activate();

    bool isActivated() const { return mActivated; }

public:
    UPROPERTY()
    UCiFKnowledge* mKnowledge; // the knowledge this plot point is based on

    // Plot points that must have been triggered before this one can be active (Story DAG representation) (hard constraint)
    UPROPERTY()
    TArray<FName> mStoryPreConditions;

    // Alternative ways this plot point can be revealed. The first completed route activates the shared plot point knowledge.
    UPROPERTY(VisibleAnywhere, Category = "CiF|Narrative")
    TArray<FCifPlotPointRevelation> mRevelations;

    // Social state pre-conditions (hard constraint) (Handled elsewhere?)
    UPROPERTY()
    TArray<UCiFRule*> mPreConditions;

    // TODO - remove?
    /*
            // Rules for when it'd be best to have the plot point show up (soft constraint) (handled elsewhere)
            public var timingIRS:InfluenceRuleSet = new InfluenceRuleSet();
    */

    // how important this plot point
    // TODO: not defined yet in what context. overall story? between the points that are pre-requisites to their next in graph?
    UPROPERTY()
    float mWeight = 0;

private:
    UPROPERTY(SaveGame)
    bool mActivated = false; // whether a plot point has been used/shown the player
};
