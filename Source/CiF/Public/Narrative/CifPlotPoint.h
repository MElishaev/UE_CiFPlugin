// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CiFKnowledge.h"
#include "CifPlotPoint.generated.h"

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

public:
    UPROPERTY()
    UCiFKnowledge* mKnowledge; // the knowledge this plot point is based on
    
	// Plot points that must have been triggered before this one can be active (Story DAG representation) (hard constraint)
	UPROPERTY()
	TArray<FName> mStoryPreConditions;

    UPROPERTY()
    TArray<FName> mRevealedBy; // array of game object that this plot point can be revealed by (items/characters etc.)

	// TODO - remove?
	/*		// Social state pre-conditions (hard constraint) (Handled elsewhere?)
			public var preconditions:Vector.<Rule> = new Vector.<Rule>(); 
	
			// Rules for when it'd be best to have the plot point show up (soft constraint) (handled elsewhere)
			public var timingIRS:InfluenceRuleSet = new InfluenceRuleSet(); 
	*/
	
	UPROPERTY()
	bool mActivated = false; // whether a plot point has been used/shown the player

	// how important this plot point
	// TODO: not defined yet in what context. overall story? between the points that are pre-requisites to their next in graph?
	UPROPERTY()
	float mWeight = 0; 
    
	FString instantiationFilePath;
};
