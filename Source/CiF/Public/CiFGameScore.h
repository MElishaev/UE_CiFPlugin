// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CiFGameScore.generated.h"

USTRUCT(BlueprintType)
struct FGameScore
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FName mName; // the name of the scored social exchange

	UPROPERTY(BlueprintReadOnly)
	FName mInitiator; // the potential initiator of the social exchange

	UPROPERTY(BlueprintReadOnly)
	FName mResponder;

	UPROPERTY(BlueprintReadOnly)
	FName mOther;

	UPROPERTY()
	int8 mScore;

	bool operator<(const FGameScore& o) const
	{
		// score in a descending order
		return mScore > o.mScore;
	}
};

