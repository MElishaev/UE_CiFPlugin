// Fill out your copyright notice in the Description page of Project Settings.


#include "Narrative/CifPlotPoint.h"

#include "CiFManager.h"
#include "CiFSubsystem.h"

UCifPlotPoint* UCifPlotPoint::loadFromJson(const TSharedPtr<FJsonObject> json, const UObject* worldContextObject)
{
    auto pp = NewObject<UCifPlotPoint>(const_cast<UObject*>(worldContextObject));
    const auto cifManager = worldContextObject->GetWorld()->GetGameInstance()->GetSubsystem<UCiFSubsystem>()->getInstance();
    if (!cifManager->isInitialized()) {
        UE_LOG(LogTemp, Error, TEXT("Trying to load plot point but cif manager not initialized - knowledge required to be loaded"
                                    "for plot point to be able to load"));
        return nullptr;
    }

    // load knowledge that this PP based on
    auto ppName = FName(json->GetStringField(TEXT("_name")));
    pp->mKnowledge = cifManager->getKnowledgeByName(ppName);
    if (!pp->mKnowledge) {
        UE_LOG(LogTemp, Error, TEXT("Didn't find knowledge %s in knowledge list in cif"), *ppName.ToString());
        return nullptr;
    }

    // load revealing character/item names
    TArray<FString> revealingObjects;
    if (json->TryGetStringArrayField(TEXT("_revealedBy"), revealingObjects)) {
        for (FString& obj : revealingObjects) {
            pp->mRevealedBy.Add(FName(obj));
        }
    }

    // load story preconditions
    TArray<FString> preconditionValues;
    if (json->TryGetStringArrayField(TEXT("_preconditions"), preconditionValues)) {
        for (FString& precondVal : preconditionValues) {
            pp->mStoryPreConditions.Add(FName(precondVal));
        }
    }
    
    pp->instantiationFilePath = json->GetStringField(TEXT("_instantiationFile"));

    return pp;
}
