// Fill out your copyright notice in the Description page of Project Settings.


#include "Narrative/CifPlotPoint.h"

#include "CiFManager.h"
#include "CiFSubsystem.h"
#include "GLSMacroses.h"

UCifPlotPoint* UCifPlotPoint::loadFromJson(const TSharedPtr<FJsonObject> json, const UObject* worldContextObject)
{
    auto pp = NewObject<UCifPlotPoint>(const_cast<UObject*>(worldContextObject));
    const auto cifManager = worldContextObject->GetWorld()->GetGameInstance()->GetSubsystem<UCiFSubsystem>()->getInstance();
    if (!cifManager->isInitialized()) {
        GLS_LOG_CONTEXT(worldContextObject,
                        LogTemp,
                        Error,
                        TEXT("Trying to load plot point but cif manager not initialized - knowledge required to be loaded"
                             "for plot point to be able to load"));
        return nullptr;
    }

    // load knowledge that this PP based on
    auto ppName = FName(json->GetStringField(TEXT("_name")));
    pp->mObjectName = ppName;
    pp->mKnowledge = cifManager->getKnowledgeByName(ppName);
    if (!pp->mKnowledge) {
        GLS_LOG_CONTEXT(worldContextObject, LogTemp, Error, TEXT("Didn't find knowledge %s in knowledge list in cif"), *ppName.ToString());
        return nullptr;
    }

    // Load the alternative ways this plot point can be revealed.
    const TArray<TSharedPtr<FJsonValue>>* revelationValues = nullptr;
    if (json->TryGetArrayField(TEXT("_revelations"), revelationValues)) {
        for (const TSharedPtr<FJsonValue>& revelationValue : *revelationValues) {
            const TSharedPtr<FJsonObject> revelationJson = revelationValue->AsObject();
            if (!revelationJson.IsValid()) {
                GLS_LOG_CONTEXT(
                    worldContextObject, LogTemp, Warning, TEXT("Ignoring invalid revelation on plot point %s"), *ppName.ToString());
                continue;
            }

            FCifPlotPointRevelation revelation;
            TArray<FString> revealingObjects;
            if (revelationJson->TryGetStringArrayField(TEXT("_revealedBy"), revealingObjects)) {
                for (const FString& revealingObject : revealingObjects) {
                    revelation.mRevealedBy.Add(FName(revealingObject));
                }
            }

            FString instantiationId;
            if (!revelationJson->TryGetStringField(TEXT("_instantiationId"), instantiationId) || instantiationId.IsEmpty()) {
                GLS_LOG_CONTEXT(worldContextObject,
                                LogTemp,
                                Warning,
                                TEXT("Ignoring revelation without an instantiation ID on plot point %s"),
                                *ppName.ToString());
                continue;
            }

            revelation.mInstantiationId = FName(instantiationId);
            pp->mRevelations.Add(MoveTemp(revelation));
        }
    }

    // load story preconditions
    TArray<FString> preconditionValues;
    if (json->TryGetStringArrayField(TEXT("_preconditions"), preconditionValues)) {
        for (FString& precondVal : preconditionValues) {
            pp->mStoryPreConditions.Add(FName(precondVal));
        }
    }

    return pp;
}

void UCifPlotPoint::activate()
{
    mActivated = true;
    GLS_LOG(LogTemp, Log, TEXT("plot point {%s} activated"), *mObjectName.ToString());
}
