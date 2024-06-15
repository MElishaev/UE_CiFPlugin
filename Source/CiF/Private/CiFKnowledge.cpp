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
