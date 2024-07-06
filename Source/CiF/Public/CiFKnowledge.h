// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CiFGameObject.h"
#include "CiFKnowledge.generated.h"

// TODO -	is knowledge being enum is enough? if i want the knowledge be dynamic,
//			than i will need to make the knowledge class much more complex.
//			for example, if i want the knowledge represent if somebody knows about
//			somebody else dating, it needs to be possible that the knowledge enum
//			represent "DATING" and the knowledge class itself should represent the
//			characters that are dating - making it more dynamic and applicable to all
//			combinations of dating states between characters, which is not supported
//			by the way it is developed now (and also in mismanor - the knowledge
//			is static and mainly STORY knowledge)
UENUM(BlueprintType)
enum class EKnowledgeType : uint8
{
	INVALID,
	KNOWLEDGE_A,
	KNOWLEDGE_B,
};

/**
 * Stores the basic information knowledge needs to operate in CiF.
 * It has a set of permanent traits, name and statuses
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CIF_API UCiFKnowledge : public UCiFGameObject
{
	GENERATED_BODY()

public:
	UCiFKnowledge();

	UFUNCTION(BlueprintCallable)
	void init(const EKnowledgeType knowledgeType, UCiFGameObject* learnedFrom=nullptr);

	static UCiFKnowledge* loadFromJson(const TSharedPtr<FJsonObject> json, const UObject* worldContextObject);
public:

	UPROPERTY(BlueprintReadWrite)
	UCiFGameObject* mLearnedFrom; // the game object that this knowledge was learned from

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	EKnowledgeType mKnowledgeType;
	
};
