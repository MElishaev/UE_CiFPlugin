// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CiFGameScore.generated.h"

USTRUCT()
struct FGameScore
{
	GENERATED_BODY()

	FName mName; // the name of the scored social exchange
	FName mInitiator; // the potential initiator of the social exchange
	FName mResponder;
	FName mOther;
	int8 mScore;

	bool operator<(const FGameScore& o) const
	{
		return mScore < o.mScore;
	}
};

