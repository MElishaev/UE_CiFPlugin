// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CifInstantiation.generated.h"

/**
 * 
 */
UCLASS()
class CIF_API UCifInstantiation : public UObject
{
    GENERATED_BODY()

public:
    // Function to assign a data table dynamically
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void assignDialogueTree(TSoftObjectPtr<UDataTable> dtRef);

    UFUNCTION()
    bool requiresOtherToPerform() { return false; }

    static UCifInstantiation* loadFromJson(const TSharedPtr<FJsonObject> json, UObject* worldContextObj);
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    UDataTable* mDialogueTable;

};
