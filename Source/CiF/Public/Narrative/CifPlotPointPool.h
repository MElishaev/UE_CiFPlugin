// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CifPlotPointPool.generated.h"

class UCifInstantiation;
class UCifPlotPoint;
enum class ETrait : uint8;
/**
 * Aggregates all the possible plot points in the game.
 * This class also tracks what plot points are available.
 * Availability of a plot point depends on its pre-requisites.
 */
UCLASS()
class CIF_API UCifPlotPointPool : public UObject
{
    GENERATED_BODY()

public:
	const UCifPlotPoint* getPlotPointByName(const FName plotPointName) const;
    
    void revealPlotPoint(const FName plotPointName);

    /**
     * checks if all story preconditions are activated, if they are, it means this plot point is available.
     * if this plot point is already activated, returns false.
     * @returns true if the story point available and false otherwise
     */
    bool isAvailableByName(const FName ppName) const;
    bool isAvailableByObj(const UCifPlotPoint* pp) const;

    /**
     * @param responder responder name
     * @return name of the plot point that is available to be revealed by the given responder, or NONE if not available
     */
    FName getAvailablePPByResponder(const FName responder) const;
    
    // todo - method to score plot point
    
    // todo - method that returns the N highest scored and available plot points
    TArray<const UCifPlotPoint*> getNHighestAvailablePoints(const uint8 n);

    // todo - method to update the state of a plot point to be discovered

/*

    // Mark selected plot point as used
	public function plotPointUsed(): void
	{
		var ggm:GrailGM = GrailGM.getInstance();
		
		// if a plot point has been selected
		if (this.selectedPlotPoint)
		{
			// mark it as activated and remove it from the active plot point list
			selectedPlotPoint.activated = true;
			ggm.plotPointFound = true;
			ggm.plotPointUsedName = selectedPlotPoint.name;
			
			var index:int = this.activePlotPoints.indexOf(selectedPlotPoint);
			
			if (index >= 0)
				this.activePlotPoints.splice(index, 1);
				
			// null it out as nothing is now selected
			this.selectedPlotPoint = null;
			
			// update plotmixing and plotconcentration weights (they change as the story progresses)
			if (mPlotMixing > 0.3)
			{
				mPlotMixing -= 2 / totalPlotPoints;
				mPlotConcentration += 2 / totalPlotPoints;
				
			}
		}
	}
		
	// update the list of active plot points
	void updateActive();
		
	// global mix-in called, insert plot point dialogue into current played instantiation if available
    // todo - search where it is called in mismanor...
	void plotPointMixIn(const FName initiator, const FName responder, const LineOfDialogue& lod, uint8 lineNumber, piece:String, Instantiation* dialog)
	{
        // TODO: Put in some logic so that plot points don't happen every move

        // search for one of the plot point string markers
        int instID = GrailInstantiation.getIDByName(piece);

        if (instID >= 0) {
            // if there are any active plot points
            if (mActivePlotPoints.Num() > 0 && GrailGM.getInstance().movesSincePlotPoint > 5) {
                // get one that matches the speaker and type of instantiation
                GrailInstantiation *instantiation = getInstantiation(responder, instID);

                // if we find one that matches the speaker and type of instantiation, mix it into the dialogue
                if (instantiation) {
                    for (const auto plod : instantiation.cifInstantiation.lines)
                        dialog.lines.splice(++lineNumber, 0, plod); // swaps 0 elements starting at (lineNumber+1) with plod

                    GrailGM.getInstance().plotPointFound = true;
                }
            }

            // update dialog with our changes
            dialog.lines.splice(dialog.lines.indexOf(lod), 1);
        }
	}
		
	void updatePool()
	{
		if (this.selectedPlotPoint && !this.selectedPlotPoint.isEndGame())
		{
			// keep track of which line the plot point was a part of
			for (var i:Number = 0; i < this.selectedPlotPoint.cifKnowledge.traits.length; i++)
			{
				if (this.selectedPlotPoint.cifKnowledge.traits[i] >= Trait.FIRST_LINE_TRAIT && 
					this.selectedPlotPoint.cifKnowledge.traits[i] <= Trait.LAST_LINE_TRAIT)
				{
					this.linesFound.push(this.selectedPlotPoint.cifKnowledge.traits[i]);
				}
			}
			// mark plot point as used
			this.plotPointUsed();
			// update the possible plot points
			this.updatePossible();
			// get a new active plot point on the roster if necessary
			this.updateActive();
		}
		else if (this.selectedPlotPoint && this.selectedPlotPoint.isEndGame())
		{
			this.plotPointUsed();
			GrailGM.getInstance().endGame = true;
		}
	
	}
	
	// go through active plot points, and find the highest weighted instantiation that matches
	UCifInstantiation* getInstantiation(const FName responder, const int type)
		{
            std::vector<UCifInstantiation *> matchingInst;
            std::vector<UCifPlotPoint *> matchingPlotPoints;

            for (const auto pp : mActivePlotPoints) {
                UCifInstantiation instantiation;

                instantiation = pp.getInstantiationBySpeakerAndType(responder, type);
                if (instantiation) {
                    matchingInst.push_back(instantiation);
                    matchingPlotPoints.push_back(pp);
                }
            }

            // TODO: Add some weighting instead of using random
            if (matchingInst.size() > 0) {
                int rnd = FMath::RandRange(0, matchingInst.size());
                selectedPlotPoint = matchingPlotPoints[rnd];
                return matchingInst[rnd];
            }
            return nullptr;
		}

*/

    static ::UCifPlotPointPool* loadFromJson(const TArray<TSharedPtr<FJsonValue>>& json, const UObject* worldContextObject);
    
private:
    /** goes over all plot points and inserts only the currently available into mAvailablePlotPoints.
     * available plot point is a point that all its pre-requisites are met. */
    void updateAvailable();

    // return the number of met pre-conditions - through the whole DAG
    int countMetPreCons(const FName ppName, const int numPreCons);
	
public:
    UPROPERTY()
    TMap<FName, UCifPlotPoint*> mPlotPoints; // all the plot points in game

    /* currently available plot points. changes after each new plot point being activated.
     * difference between available and active is that the available are the all plot points
     * that currently have their pre-requisites met - but active plot points are based on designer's
     * choice, the N highest scored available points. This means that the game may "land" on the
     * player one of the active plot points - making the point "activated", aka known to the player.
     */
    UPROPERTY()
    TArray<UCifPlotPoint*> mAvailablePlotPoints;
    TArray<UCifPlotPoint*> mActivePlotPoints; // empty list means none active
    // public var selectedPlotPoint:PlotPoint;
		
    // weights
    UPROPERTY()
    float mStoryCohesion = 0.1; // how much the plot point is related to the other ones unlocked

    UPROPERTY()
    float mPlotMixing = 0.6; // how different are the plot points' plot lines at the beginning of the game

    UPROPERTY()
    float mPlotConcentration = 0.3; // how similar are the plot points' plot lines at the end of the game
    
    TArray<ETrait> linesFound;
    int totalPlotPoints;
		
    // keeps track of how long since last plot point - used for weighting quest availability
    int movesSinceLast = 0; 
};
