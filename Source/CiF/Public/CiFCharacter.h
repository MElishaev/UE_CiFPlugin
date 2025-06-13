// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CiFGameObject.h"
#include "CiFCharacter.generated.h"

enum class ECiFItemType : uint8;
enum class EItemType : uint8;
enum class EKnowledgeType : uint8;
class UCiFItem;
class UCiFKnowledge;
class UCiFProspectiveMemory;

/* Types */
UENUM(BlueprintType)
enum class ECiFGender : uint8
{
	MALE		UMETA(DisplayName="Male"),
	FEMALE		UMETA(DisplayName="Female")
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
 */
UCLASS(meta=(BlueprintSpawnableComponent))
class CIF_API UCiFCharacter : public UCiFGameObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "worldContextObject"), Category="CiFCharacter Methods")
	void init(UObject* worldContextObject);

	/**
	 * @param knowledge The knowledge to query for
	 * @param learnedFrom The character this knowledge was learned from - nullptr if this doesn't matter for the query
	 * @return True if this knowledge known to the character
	 */
	UFUNCTION(BlueprintCallable, Category="CiFCharacter Methods")
	bool hasKnowledge(const UCiFKnowledge* knowledge, const UCiFCharacter* learnedFrom=nullptr) const;	
	
	/**
	 * @param item The item to query for
	 * @param receivedFrom The character this item was received from - nullptr if this doesn't matter for the query
	 * @return True if character has this item
	 */
	UFUNCTION(BlueprintCallable, Category="CiFCharacter Methods")
	bool hasItem(const UCiFItem* item, const UCiFCharacter* receivedFrom=nullptr) const;

	UFUNCTION(BlueprintCallable, Category="CiFCharacter Methods")
	void addKnowledge(const EKnowledgeType knowledgeType, UCiFGameObject* learnedFrom=nullptr);

	UFUNCTION(BlueprintCallable, Category="CiFCharacter Methods")
	void addItem(const ECiFItemType itemType, UCiFGameObject* recievedFrom=nullptr);
	
	UFUNCTION(BlueprintCallable, Category="CiFCharacter Methods")
	void removeItem(const ECiFItemType itemType);

	void resetProspectiveMemory();

	static UCiFCharacter* loadFromJson(TSharedPtr<FJsonObject> json, const UObject* worldContextObject);
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CiFCharacter Properties")
	TMap<EKnowledgeType, UCiFKnowledge*> mKnowledgeMap; // the knowledge known by the character

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CiFCharacter Properties")
	TMap<ECiFItemType, UCiFItem*> mItemMap; // the items held by the character

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CiFCharacter Properties")
	ECiFGender mGender;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="CiFCharacter Properties")
	UCiFProspectiveMemory* mProspectiveMemory; // prospective memory of the character

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CiFCharacter Properties")
	TMap<ELocutionType, FText> mDefaultLocutions; // TODO-initialize with default values - this is relevant only if your game realizes the CiF with dialogue
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CiFCharacter Properties")
	TMap<ELocutionType, FText> mLocutions; // character specific mix-ins that are used in performance realization
};
