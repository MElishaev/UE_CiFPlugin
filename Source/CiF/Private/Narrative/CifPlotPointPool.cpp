// Fill out your copyright notice in the Description page of Project Settings.


#include "Narrative/CifPlotPointPool.h"
#include "Narrative/CifPlotPoint.h"

const UCifPlotPoint *UCifPlotPointPool::getPlotPointByName(const FName plotPointName) const
{
    if (const auto pp = mPlotPoints.Find(plotPointName)) {
        return *pp;
    }
    return nullptr;
}

int UCifPlotPointPool::countMetPreCons(const FName ppName, const int numPreCons)
{
    int result = numPreCons;
    if (auto pp = getPlotPointByName(ppName)) {
        for (const auto &preconName : pp->mStoryPreConditions) {
            if (const auto precon = getPlotPointByName(preconName)) {
                if (precon->mActivated) {
                    result++;
                    result = countMetPreCons(ppName, result);    
                }
            }
            
        }   
    }
    return result;
}

bool UCifPlotPointPool::isAvailableByName(const FName ppName) const
{
    if (const auto pp = getPlotPointByName(ppName)) {
        return isAvailableByObj(pp);
    }

    return false;
}

bool UCifPlotPointPool::isAvailableByObj(const UCifPlotPoint* pp) const
{
    // if it's already been activated, then not available
    if (pp->mActivated) {
        return false;
    }

    for (const auto preConName : pp->mStoryPreConditions) {
        if (const auto preCon = getPlotPointByName(preConName)) {
            if (!preCon->mActivated) {
                return false;
            }    
        }
    }
    return true;
}

FName UCifPlotPointPool::getAvailablePPByResponder(const FName responder) const
{
    for (const auto pp: mAvailablePlotPoints) {
        if (pp->mRevealedBy.Contains(responder)) {
            return pp->mKnowledge->mObjectName;
        }
    }
    return NAME_None;
}

TArray<const UCifPlotPoint *> UCifPlotPointPool::getNHighestAvailablePoints(const uint8 n)
{
    return {};
}
/*
void UCifPlotPointPool::updateActive()
{
	// TODO: Maybe shouldn't clear it out every time? If not, then when making possible plot points, check if they're in active so you don't get duplicates
	this.activePlotPoints = new Vector.<PlotPoint>();

    // go through available plot points and activate (???) the N highly scored ones todo is this how we should do it? 
	for (const auto pp : mAvailablePlotPoints) {
	    // Story Cohesion - Higher weights on story elements with the most pre-reqs met
	    // count the pre-conditions of the plot points, adding one point for every pre-condition met.
	    float cohesion = pp->countMetPreCons();
	    cohesion *= mStoryCohesion;
		
	    float mixing = 0;
	    float concentration = 0;
		
	    // Storyline Mixing - Higher weights on plot points from different LINE traits (stronger weights at the beginning of the game)
	    // Keep track of which *_LINE plot points have been coming from. 
	    // Storyline Concentration - Higher weights on plot points from same LINE traits (stronger weights at the end of the game)
	    // Keep track of which *_LINE plot points have been coming from. 1 point if covered last turn, .5 in 2 turns, .25 in 3 turns.
		
	    TArray<ETrait> traitLines;
        for (const auto trait : pp->mTraits) {
            if (trait > ETrait::FIRST_LINE_TRAIT && trait < ETrait::LAST_LINE_TRAIT) {
                traitLines.Add(trait);

                // see if line is already in linesFound and keep track of the position
                int position = 0;
                for (auto line : linesFound) {
                    if (line == trait) {
                        mixing += FMath::Pow(0.5, position);
                    }
                    position++;
                }
            }
        }
	    concentration = mixing; // mixing and concentration are similar but opposite (that's why below multiplying by -1)
	    
	    // normalize for number of *_LINE traits
	    mixing = mixing / traitLines.Num();
	    // negative weight - because mixing in this method accumulates positive values when we encounter similar plot
	    // lines but we want to positively value when encountering different plot lines at the beginning of the game
	    mixing *= -1; 
	    mixing *= mPlotMixing;
		
	    concentration = concentration / traitLines.Num();
	    concentration *= mPlotConcentration;
		
	    pp->mWeight = cohesion + mixing + concentration;
	}

	// returning -1, 0, 1
    auto plotPointSort = [](const UCifPlotPoint *a, const UCifPlotPoint *b) -> int {
        if (a->mWeight > b->mWeight)
            return -1;
        if (a->mWeight < b->mWeight)
            return 1;
        return 0;
    };
	
	mAvailablePlotPoints.Sort(plotPointSort);
	
	// take the top 3 highest rated plot points
	while (mActivePlotPoints.Num() < 3 && mAvailablePlotPoints.Num() > 0) {
        mActivePlotPoints.Add(mAvailablePlotPoints[0]);
        mAvailablePlotPoints.RemoveAt(0);
    }
}
*/

UCifPlotPointPool* UCifPlotPointPool::loadFromJson(const TArray<TSharedPtr<FJsonValue>>& json, const UObject* worldContextObject)
{
    const auto ppPool = NewObject<UCifPlotPointPool>(const_cast<UObject*>(worldContextObject));

    for (const auto ppJson : json) {
        auto pp = UCifPlotPoint::loadFromJson(ppJson->AsObject(), worldContextObject);
        ppPool->mPlotPoints.Add(pp->mKnowledge->mObjectName, pp);
    }

    return ppPool;
}

void UCifPlotPointPool::updateAvailable()
{
    mAvailablePlotPoints.Empty();

    for (const auto [name, pp] : mPlotPoints) {
        if (!pp->mActivated) {
            // check pre-reqs
            if (isAvailableByObj(pp)) {
                mAvailablePlotPoints.Add(pp);
            }
        }
    }
}		
