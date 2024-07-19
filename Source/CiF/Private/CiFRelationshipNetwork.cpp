// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFRelationshipNetwork.h"
#include "CiFCharacter.h"
#include "CiFManager.h"
#include "CiFSubsystem.h"

void UCiFRelationshipNetwork::initialize(const uint8 numOfCharacters, const uint8 maxVal)
{
	Super::init(ESocialNetworkType::RELATIONSHIP, numOfCharacters, maxVal);
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

UCiFRelationshipNetwork* UCiFRelationshipNetwork::loadFromJson(const TSharedPtr<FJsonObject> json, const UObject* worldContextObject)
{
	const auto rn = NewObject<UCiFRelationshipNetwork>(const_cast<UObject*>(worldContextObject));

	const auto numChars = json->GetNumberField("_numChars");
	const int8 maxVal = 100;
	rn->initialize(numChars, maxVal);

	const auto cifManager = rn->GetWorld()->GetGameInstance()->GetSubsystem<UCiFSubsystem>()->getInstance();
	const auto rsJson = json->GetArrayField("Relationships");
	for (const auto rJson : rsJson) {
		const auto rnEnum = StaticEnum<ERelationshipType>();
		const auto rnType = static_cast<ERelationshipType>(rnEnum->GetValueByName(FName(json->GetStringField("_type"))));
		const auto from = FName(rJson->AsObject()->GetStringField("_from"));
		const auto to = FName(rJson->AsObject()->GetStringField("_to"));
		const auto fromObject = static_cast<UCiFCharacter*>(cifManager->getGameObjectByName(from));
		const auto toObject = static_cast<UCiFCharacter*>(cifManager->getGameObjectByName(to));
		if (fromObject && toObject) {
			rn->setRelationship(rnType, fromObject, toObject);
		}
		else {
			UE_LOG(LogTemp, Error, TEXT("Couldn't find the characters that represented by relationship"));
		}
	}
	
	return rn;
}
