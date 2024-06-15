// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFItem.h"


// Sets default values for this component's properties
UCiFItem::UCiFItem()
{
	PrimaryComponentTick.bCanEverTick = false;
	mGameObjectType = ECiFGameObjectType::ITEM;
	mItemType = ECiFItemType::INVALID;
	mReceivedFrom = nullptr;
}

void UCiFItem::init(const ECiFItemType itemType, UCiFGameObject* receivedFrom)
{
	mItemType = itemType;
	mReceivedFrom = receivedFrom;
}
