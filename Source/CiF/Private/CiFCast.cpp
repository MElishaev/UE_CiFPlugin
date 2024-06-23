// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFCast.h"
#include "CiFCharacter.h"

UCiFCharacter* UCiFCast::getCharByName(const FName name) const
{
	auto character = mCharactersByName.Find(name);
	if (character)
	{
		return *character;
	}
	return nullptr;
}

UCiFCharacter* UCiFCast::getCharByNetworkId(const uint8 id) const
{
	for (auto c : mCharacters)
	{
		if (c->mNetworkId == id)
		{
			return c;
		}
	}
	return nullptr;
}

void UCiFCast::addCharacter(UCiFCharacter* c)
{
	mCharacters.AddUnique(c);
	mCharactersByName[c->mObjectName] = c;
}
