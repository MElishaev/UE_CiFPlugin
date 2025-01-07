// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Utilities.h"
#include "CiFGameScore.generated.h"

USTRUCT(BlueprintType)
struct FGameScore
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="CiF")
	FName mName; // the name of the scored social exchange

	UPROPERTY(BlueprintReadOnly, Category="CiF")
	FName mInitiator; // the potential initiator of the social exchange

	UPROPERTY(BlueprintReadOnly, Category="CiF")
	FName mResponder;

	UPROPERTY(BlueprintReadOnly, Category="CiF")
	FName mOther;

	UPROPERTY(BlueprintReadOnly, Category="CiF")
	FScore_t mScore;

	bool operator<(const FGameScore& o) const
	{
		// score in a descending order
		return mScore > o.mScore;
	}
};

