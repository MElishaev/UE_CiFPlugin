// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFRelationshipNetwork.h"
#include "CiFCharacter.h"

void UCiFRelationshipNetwork::initialize(const uint8 numOfCharacters, const uint8 maxVal)
{
	Super::init(ESocialNetworkType::RN_RELATIONSHIP, numOfCharacters, maxVal);
	setAllArrayElements(0);
}

bool UCiFRelationshipNetwork::getRelationship(const ERelationshipType relationship, const UCiFCharacter* a, const UCiFCharacter* b)
{
	return ((1u << static_cast<uint8>(relationship)) & getWeight(a->mNetworkId, b->mNetworkId)) > 0;
}

void UCiFRelationshipNetwork::removeRelationship(const ERelationshipType relationship, const UCiFCharacter* a, const UCiFCharacter* b)
{
	const int val = getWeight(a->mNetworkId, b->mNetworkId) & ~(1 << static_cast<uint8>(relationship));
	setWeight(a->mNetworkId, b->mNetworkId, val);
	setWeight(b->mNetworkId, a->mNetworkId, val);
}

void UCiFRelationshipNetwork::setRelationship(const ERelationshipType relationship, const UCiFCharacter* a, const UCiFCharacter* b)
{
	const uint8 val = getWeight(a->mNetworkId, b->mNetworkId) | (1u << static_cast<uint8>(relationship));
	setWeight(a->mNetworkId, b->mNetworkId, val);
	setWeight(b->mNetworkId, a->mNetworkId, val);
}
