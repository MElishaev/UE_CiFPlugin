// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFCharacter.h"

#include "CiFGameObjectStatus.h"
#include "CiFItem.h"
#include "CiFKnowledge.h"
#include "CiFManager.h"
#include "CiFProspectiveMemory.h"
#include "CiFSubsystem.h"

void UCiFCharacter::init()
{
	mGameObjectType = ECiFGameObjectType::CHARACTER;
}

bool UCiFCharacter::hasKnowledge(const UCiFKnowledge* knowledge, const UCiFCharacter* learnedFrom) const
{
	auto k = *(mKnowledgeMap.Find(knowledge->mKnowledgeType));
	if (k)
	{
		if (learnedFrom)
		{
			return k->mLearnedFrom == learnedFrom;	
		}
		return true;
	}
	return false;
}

bool UCiFCharacter::hasItem(const UCiFItem* item, const UCiFCharacter* receivedFrom) const
{
	auto i = *(mItemMap.Find(item->mItemType));
	if (i)
	{
		if (receivedFrom)
		{
			return i->mReceivedFrom == receivedFrom;	
		}
		return true;
	}
	return false;
}

void UCiFCharacter::addKnowledge(const EKnowledgeType knowledgeType, UCiFGameObject* learnedFrom)
{
	auto knowledge = NewObject<UCiFKnowledge>();
	knowledge->init(knowledgeType, learnedFrom);
	mKnowledgeMap.FindOrAdd(knowledgeType, knowledge);
}

void UCiFCharacter::addItem(const ECiFItemType itemType, UCiFGameObject* recievedFrom)
{
	auto item = NewObject<UCiFItem>();
	item->init(itemType, recievedFrom);
	mItemMap.FindOrAdd(itemType, item);
}

void UCiFCharacter::removeItem(const ECiFItemType itemType)
{
	mItemMap.Remove(itemType);
}

void UCiFCharacter::resetProspectiveMemory()
{
	mProspectiveMemory = NewObject<UCiFProspectiveMemory>();
}

UCiFCharacter* UCiFCharacter::loadFromJson(TSharedPtr<FJsonObject> json, const UObject* worldContextObject)
{
	const auto c = NewObject<UCiFCharacter>(const_cast<UObject*>(worldContextObject));
	c->init();

	c->UCiFGameObject::loadFromJson(json, worldContextObject);
	
	return c;
}
