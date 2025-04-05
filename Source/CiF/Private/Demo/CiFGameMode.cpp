// Fill out your copyright notice in the Description page of Project Settings.


#include "Demo/CiFGameMode.h"

#include "CiFManager.h"
#include "CiFSubsystem.h"
#include "Demo/CifNPC.h"
#include "Demo/DemoCifImplementation.h"

void ACiFGameMode::BeginPlay()
{
    Super::BeginPlay();
    initCif();
    spawnCastInLevel();
}

void ACiFGameMode::spawnCastInLevel_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("%hs: In cpp"), __FUNCTION__);
}


void ACiFGameMode::initCif()
{
    const auto cifSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UCiFSubsystem>();
    mCifManager = cifSubsystem->getInstance();
    mCifManager->init(GetWorld());
    cifSubsystem->setImplementation(NewObject<UDemoCifImplementation>());
}

ACifNPC* ACiFGameMode::getCifNPCByName(FName name)
{
    const auto npc = mCifNPCs.FindByPredicate([=](const ACifNPC* elem) {
        return elem->mCifCharacterComp->mObjectName == name;
    });

    return npc ? *npc : nullptr;
}
