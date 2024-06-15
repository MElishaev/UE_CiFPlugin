// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CiFGameObject.h"
#include "CiFCharacter.generated.h"

enum class EItemType : uint8;
enum class EKnowledgeType : uint8;
class UCiFItem;
class UCiFKnowledge;
class UCiFProspectiveMemory;

/* Types */
UENUM(BlueprintType)
enum class EGender : uint8
{
	MALE		UMETA(DisplayName="Male"),
	FEMALE	UMETA(DisplayName="Female")
};

UENUM(BlueprintType)
enum class ETraits : uint8
{
	BRAVE		UMETA(DisplayName="Brave"),
	SMART		UMETA(DisplayName="Smart"),
	SIZE
};

UENUM()
enum class ELocutionType
{
	GREETING,
	SHOCKED,
	FRIENDLY,
};

/**
 * A CiF object derived class representing characters.
 * TODO: maybe add functionality to construct characters from json properties for modding community
 */
UCLASS()
class CIF_API UCiFCharacter : public UCiFGameObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	void init();

	/**
	 * @param knowledge The knowledge to query for
	 * @param learnedFrom The character this knowledge was learned from - nullptr if this doesn't matter for the query
	 * @return True if this knowledge known to the character
	 */
	UFUNCTION(BlueprintCallable)
	bool hasKnowledge(const UCiFKnowledge* knowledge, const UCiFCharacter* learnedFrom=nullptr) const;	
	
	/**
	 * @param item The item to query for
	 * @param receivedFrom The character this item was received from - nullptr if this doesn't matter for the query
	 * @return True if character has this item
	 */
	UFUNCTION(BlueprintCallable)
	bool hasItem(const UCiFItem* item, const UCiFCharacter* receivedFrom=nullptr) const;

	UFUNCTION(BlueprintCallable)
	void addKnowledge(const EKnowledgeType knowledgeType, UCiFGameObject* learnedFrom=nullptr);

	UFUNCTION(BlueprintCallable)
	void addItem(const ECiFItemType itemType, UCiFGameObject* recievedFrom=nullptr);
	
	UFUNCTION(BlueprintCallable)
	void removeItem(const ECiFItemType itemType);
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EKnowledgeType, UCiFKnowledge*> mKnowledgeMap; // the knowledge known by the character

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<ECiFItemType, UCiFItem*> mItemMap; // the items owned by the character

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EGender mGender;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UCiFProspectiveMemory* mProspectiveMemory; // prospective memory of the character

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<ELocutionType, FText> mDefaultLocutions; // TODO-initialize with default values - this is relevant only if your game realizes the CiF with dialogue
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<ELocutionType, FText> mLocutions; // character specific mix-ins that are used in performance realization
};
