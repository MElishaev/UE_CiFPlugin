// Fill out your copyright notice in the Description page of Project Settings.


#include "Narrative/CifNarrativeManager.h"
#include "Narrative/CifPlotPointPool.h"
#include "ReadWriteFiles.h"
#include "Narrative/MK_DialogueManager.h"

void UCifNarrativeManager::init()
{
    const FString plotpointsPath = FPaths::Combine(*FPaths::ProjectPluginsDir(), *FString("CiF/Content/Data/plotpoints.json"));
    UE_LOG(LogTemp, Log, TEXT("Reading plot points from %s"), *plotpointsPath);
    loadPlotPoints(plotpointsPath, this);

    mDialogueMgr = NewObject<UMK_DialogueManager>();
    mDialogueMgr->initializeRegistry(FPaths::Combine(*FPaths::ProjectPluginsDir(),
                                                     *FString("CiF/Content/Data/Dialogue/dialogue_registry.json")));
}

void UCifNarrativeManager::loadPlotPoints(const FString& filePath, const UObject* worldContextObject)
{
    TSharedPtr<FJsonObject> jsonObject;
    if (!UReadWriteFiles::readJson(filePath, jsonObject)) {
        UE_LOG(LogTemp, Error, TEXT("Failed to read json %s"), *filePath);
        return;
    }

    const auto ppJson = jsonObject->GetArrayField(TEXT("Plotpoints"));
    mPlotPointPool = UCifPlotPointPool::loadFromJson(ppJson, worldContextObject);
    if (!mPlotPointPool) {
        UE_LOG(LogTemp, Error, TEXT("Failed to create plot point pool"));
    }
}

FName UCifNarrativeManager::getPPNameToBePlayed(const FName responder) const
{
    return mPlotPointPool->getAvailablePPByResponder(responder);
}

void UCifNarrativeManager::getInstantiationForSocialGame(const FName sgName,
                                                         const FName initiator,
                                                         const FName responder,
                                                         const FName other)
{
}

void UCifNarrativeManager::getInstantiationForPlotPoint(const FName ppName, const FName participant)
{
}
