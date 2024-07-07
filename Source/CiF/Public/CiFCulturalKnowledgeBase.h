// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CiFCKBEntry.h"
#include "UObject/NoExportTypes.h"
#include "CiFCulturalKnowledgeBase.generated.h"

class UCiFCKBEntry;

/**
 * Holds the subjective knowledge (e.g. "Adam likes ice creams") and the general truth in the world (e.g. "Stealing is bad").
 * While something considered to be bad, that doesnt mean characters won't do it or like it.
 *
 * Example for a query:
 * CKB(GameObject, (GameObject, FSubjectiveLabel), (GameObject, FSubjectiveLabel), FTruthLabel) - general structure.
 * if we look more specific, we can get this:
 * CKB(item,(x, likes), (y, dislikes), lame)
 * This entry describes what characters x,y thinks about some item, and what is the general truth about this item, is it cool or lame.
 */
UCLASS()
class CIF_API UCiFCulturalKnowledgeBase : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * returns the character's match for liking/disliking all items that match the label 
	 */
	UFUNCTION()
	void findItems(const FName character,
	               TArray<FName>& outputItems,
	               const ESubjectiveLabel connectionTyp = ESubjectiveLabel::INVALID,
	               const ETruthLabel label = ETruthLabel::INVALID) const;


	static UCiFCulturalKnowledgeBase* loadFromJson(const TSharedPtr<FJsonObject> json, const UObject* worldContextObject);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UCiFCKBEntry*> mSubjectiveEntries;
	// example: john likes pizza //TODO: maybe think of a better structure for faster lookup of knowledge

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UCiFCKBEntry*> mGeneralTruthEntries; // example: pizza is tasty
};
