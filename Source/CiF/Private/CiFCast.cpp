// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFCast.h"
#include "CiFCharacter.h"

void UCiFCast::init(UObject* worldContextObject)
{
	for (const auto c : mCharacters) {
		c->init(worldContextObject);
	}
}

UCiFCharacter* UCiFCast::getCharByName(const FName name) const
{
	auto character = mCharactersByName.Find(name);
	if (character) {
		return *character;
	}
	return nullptr;
}

UCiFCharacter* UCiFCast::getCharByNetworkId(const uint8 id) const
{
	for (auto c : mCharacters) {
		if (c->mNetworkId == id) {
			return c;
		}
	}
	return nullptr;
}

void UCiFCast::addCharacter(UCiFCharacter* c)
{
	mCharacters.AddUnique(c);
	mCharactersByName.Add(c->mObjectName, c);
}

UCiFCast* UCiFCast::loadFromJson(const TSharedPtr<FJsonObject> json, const UObject* worldContextObject)
{
	const auto cast = NewObject<UCiFCast>(const_cast<UObject*>(worldContextObject));

	const auto castJson = json->GetArrayField("Cast");
	for (const auto charJson : castJson) {
		const auto c = UCiFCharacter::loadFromJson(charJson->AsObject(), worldContextObject);
		cast->addCharacter(c);
	}

	// after loaded all of the characters we can populate them with statuses, 
	// doing it while parsing characters is crashing because some statuses can
	// depend on characters not loaded yet.
	// for (const auto charJson : castJson) {
	// 	const FName charName(charJson->AsObject()->GetStringField("_name"));
	// 	const auto c = cast->getCharByName(charName);
	// 	const auto statusesJson = charJson->AsObject()->GetArrayField("Status");
	// 	for (const auto statusJson : statusesJson) {
	// 		const auto statusEnum = StaticEnum<EStatus>();
	// 		const auto statusType = static_cast<EStatus>(statusEnum->
	// 			GetValueByName(FName(statusJson->AsObject()->GetStringField("_type"))));
	// 		const FName towardsName(statusJson->AsObject()->GetStringField("_to"));
	// 		c->addStatus(statusType, 5, towardsName); // TODO - why the status in the json doesn't have duration?
	// 	}
	// }

	// TODO - skipping on locutions for now
	// TODO - skipping on location for now
	// TODO - skipping on inventory for now
	return cast;
}
