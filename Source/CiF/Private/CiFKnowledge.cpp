// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFKnowledge.h"


// Sets default values for this component's properties
UCiFKnowledge::UCiFKnowledge()
{
	PrimaryComponentTick.bCanEverTick = false;
	mGameObjectType = ECiFGameObjectType::KNOWLEDGE;
	mLearnedFrom = nullptr;
	mKnowledgeType = EKnowledgeType::INVALID;
}

void UCiFKnowledge::init(const EKnowledgeType knowledgeType, UCiFGameObject* learnedFrom)
{
	mLearnedFrom = learnedFrom;
	mKnowledgeType = knowledgeType;
}

UCiFKnowledge* UCiFKnowledge::loadFromJson(const TSharedPtr<FJsonObject> json, const UObject* worldContextObject)
{
	const auto knowledge = NewObject<UCiFKnowledge>(const_cast<UObject*>(worldContextObject));
	// TODO - for now im not yet sure how to handle the knowledge types etc, it seems
	// to be very long enum names - ill live it like that and fill it when i'll have
	// more concrete story or architecture in mind
	knowledge->init(EKnowledgeType::KNOWLEDGE_A);

	knowledge->UCiFGameObject::loadFromJson(json, worldContextObject);

	return knowledge;
}
