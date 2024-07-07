// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CiFCKBEntry.generated.h"

UENUM(BlueprintType)
enum class ECKBLabelType : uint8
{
	INVALID,
	GENERAL_TRUTH,
	SUBJECTIVE,
	SIZE
};

UENUM()
enum class ETruthLabel
{
	INVALID,
	ROMANTIC,
	COOL,
	GROSS,
	FUNNY,
	MEAN,
	NICE,
	RUDE,
	CHEATING,
	PRETTY,
};

UENUM()
enum class ESubjectiveLabel : uint8
{
	INVALID,
	LIKES,
	DISLIKES,
	WANTS,
	HAS,
	ARE,
};

UCLASS()
class CIF_API UCiFCKBEntry : public UObject
{
	GENERATED_BODY()

public:
	// TODO - this may need some changes because the structure is not really neat
	void initSubjectiveEntry(const FName head, const ESubjectiveLabel connectionType, const FName tail);
	void initTruthEntry(const FName head, const ESubjectiveLabel connectionType, const ETruthLabel tail);

	ECKBLabelType mType; // subjective or general truth connection
	FName mHead;         // can represent characters or nouns
	FName mTail;         // can represent nouns (/items) or truth labels
	ESubjectiveLabel mConnection;
};
