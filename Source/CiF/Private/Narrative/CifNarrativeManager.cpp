// Fill out your copyright notice in the Description page of Project Settings.


#include "Narrative/CifNarrativeManager.h"
#include "GLSMacroses.h"
#include "Narrative/CifPlotPoint.h"
#include "Narrative/CifPlotPointPool.h"
#include "Narrative/MK_DialogueManager.h"
#include "ReadWriteFiles.h"

void UCifNarrativeManager::init()
{
    const FString plotpointsPath = FPaths::Combine(*FPaths::ProjectPluginsDir(), *FString("CiF/Content/Data/plotpoints.json"));
    GLS_LOG(LogTemp, Log, TEXT("Reading plot points from %s"), *plotpointsPath);
    loadPlotPoints(plotpointsPath, this);

    mDialogueMgr = NewObject<UMK_DialogueManager>();
    const FString registryPath =
        FPaths::Combine(*FPaths::ProjectPluginsDir(), *FString("CiF/Content/Data/Dialogue/dialogue_registry.json"));
    if (!mDialogueMgr->initializeRegistry(registryPath)) {
        GLS_LOG(LogTemp, Error, TEXT("Failed to initialize dialogue registry from %s"), *registryPath);
    }
}

void UCifNarrativeManager::loadPlotPoints(const FString& filePath, const UObject* worldContextObject)
{
    TSharedPtr<FJsonObject> jsonObject;
    if (!UReadWriteFiles::readJson(filePath, jsonObject)) {
        GLS_LOG(LogTemp, Error, TEXT("Failed to read json %s"), *filePath);
        return;
    }

    const auto ppJson = jsonObject->GetArrayField(TEXT("Plotpoints"));
    mPlotPointPool = UCifPlotPointPool::loadFromJson(ppJson, worldContextObject);
    if (!mPlotPointPool) {
        GLS_LOG(LogTemp, Error, TEXT("Failed to create plot point pool"));
    }
}

bool UCifNarrativeManager::findPlotPointToPlay(const FName revealer, FCifPlotPointSelection& outSelection) const
{
    outSelection = {};
    if (!mPlotPointPool) {
        GLS_LOG(LogTemp, Error, TEXT("Cannot find a plot point to play because the plot-point pool is not initialized"));
        return false;
    }
    return mPlotPointPool->findAvailableRevelation(revealer, outSelection);
}

void UCifNarrativeManager::getInstantiationForSocialGame(const FName sgName,
                                                         const FName initiator,
                                                         const FName responder,
                                                         const FName other)
{
}

UCifInstantiation* UCifNarrativeManager::getInstantiationForPlotPoint(const FCifPlotPointSelection& selection)
{
    if (!selection.isValid()) {
        GLS_LOG(LogTemp, Error, TEXT("Cannot resolve an instantiation from an invalid plot-point selection"));
        return nullptr;
    }
    if (!mDialogueMgr) {
        GLS_LOG(LogTemp, Error, TEXT("Cannot resolve a plot-point instantiation because the dialogue manager is not initialized"));
        return nullptr;
    }

    const FName instantiationId = selection.mRevelation->mInstantiationId;
    if (instantiationId.IsNone()) {
        GLS_LOG(LogTemp, Error, TEXT("Selected plot-point revelation does not specify an instantiation ID"));
        return nullptr;
    }

    UCifInstantiation* instantiation = mDialogueMgr->prepareDialogue(instantiationId);
    if (!instantiation) {
        GLS_LOG(LogTemp, Error, TEXT("Failed to prepare plot-point instantiation %s"), *instantiationId.ToString());
        return nullptr;
    }

    mPendingPlotPointsByInstantiation.Add(instantiationId, selection.mPlotPoint->mKnowledge->mObjectName);
    return instantiation;
}

bool UCifNarrativeManager::completePlotPointDialogue(const FName instantiationName)
{
    if (!mPlotPointPool) {
        GLS_LOG(LogTemp, Error, TEXT("Cannot complete plot-point dialogue because the plot-point pool is not initialized"));
        return false;
    }

    const FName* plotPointName = mPendingPlotPointsByInstantiation.Find(instantiationName);
    if (!plotPointName) {
        GLS_LOG(LogTemp, Warning, TEXT("Dialogue %s is not pending a plot-point revelation"), *instantiationName.ToString());
        return false;
    }
    if (!mPlotPointPool->isAvailableByName(*plotPointName)) {
        GLS_LOG(LogTemp, Warning, TEXT("Plot point %s is no longer available for completion"), *plotPointName->ToString());
        return false;
    }

    mPlotPointPool->revealPlotPoint(*plotPointName);
    mPendingPlotPointsByInstantiation.Remove(instantiationName);
    return true;
}
