// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CiFGameMode.generated.h"

class UCiFManager;
class ACifNPC;
/**
 * A game mode for games using CiF system
 */
UCLASS(Blueprintable, BlueprintType)
class CIF_API ACiFGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

    virtual void BeginPlay() override;
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Default")
    void spawnCastInLevel();

    UFUNCTION(BlueprintCallable, Category="Default")
    void initCif();

    UFUNCTION(BlueprintCallable)
    ACifNPC* getCifNPCByName(FName name);

public:
    
    UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default")
    UCiFManager* mCifManager;

    UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default")
    TArray<ACifNPC*> mCifNPCs;

};
