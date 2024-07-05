// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFItem.h"


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

UCiFItem* UCiFItem::loadFromJson(const TSharedPtr<FJsonObject> json, const UObject* worldContextObject)
{
	const auto item = NewObject<UCiFItem>(const_cast<UObject*>(worldContextObject));

	const auto itemEnum = StaticEnum<ECiFItemType>();
	const FName typeName(json->GetStringField("_type"));
	const ECiFItemType itemType = static_cast<ECiFItemType>(itemEnum->GetValueByName(typeName));

	item->init(itemType);

	item->UCiFGameObject::loadFromJson(json, worldContextObject);

	return item;
}
