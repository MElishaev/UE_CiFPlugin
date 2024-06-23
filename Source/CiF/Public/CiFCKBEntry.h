// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CiFCKBEntry.generated.h"

UENUM(BlueprintType)
enum class ECKBLabelType : uint8
{
	GENERAL_TRUTH,
	SUBJECTIVE,
	SIZE
};

USTRUCT(BlueprintType)
struct FTruthLabel
{
	GENERATED_BODY()
	inline static const FName ROMANTIC = "romantic";
	inline static const FName COOL = "cool";
	inline static const FName GROSS = "gross";
	inline static const FName FUNNY = "funny";
	inline static const FName MEAN = "mean";
	inline static const FName NICE = "nice";
	inline static const FName RUDE = "rude";
	inline static const FName CHEATING = "cheating"; 
};

USTRUCT(BlueprintType)
struct FSubjectiveLabel
{
	GENERATED_BODY()
	inline static const FName LIKES = "likes";
	inline static const FName DISLIKES = "dislikes";
	inline static const FName HAS = "has";
	inline static const FName ARE = "are";
};


/**
 * Example:
 * CKB(GameObject, (GameObject, FSubjectiveLabel), (GameObject, FSubjectiveLabel), FTruthLabel) - general structure.
 * if we look more specific, we can get this:
 * CKB(item,(x, likes), (y, dislikes), lame)
 * This entry describes what characters x,y thinks about some item, and what is the general truth about this item, is it cool or lame.
 * todo: the structure of entry is still subject to change as can be seen by the above examples
 * which aren't inline with the class below.
 */
UCLASS()
class CIF_API UCiFCKBEntry : public UObject
{
	GENERATED_BODY()

public:

	void init(const FName head, const FName connectionType, const FName tail);
	
	ECKBLabelType mType; // subjective or general truth connection
	FName mHead; // can represent characters or nouns
	FName mTail; // can represent characters or truth labels
	FName mConnection;
};
