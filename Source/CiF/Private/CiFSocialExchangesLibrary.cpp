// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFSocialExchangesLibrary.h"

#include "CiFSocialExchange.h"
#include "ReadWriteFiles.h"
#include "Json.h"

void UCiFSocialExchangesLibrary::addSocialExchange(UCiFSocialExchange* se)
{
	mSocialExchanges[se->mName] = se;
}

void UCiFSocialExchangesLibrary::removeSocialExchange(UCiFSocialExchange* se)
{
	mSocialExchanges.Remove(se->mName);
}

UCiFSocialExchange* UCiFSocialExchangesLibrary::getSocialExchangeByName(const FName name)
{
	auto se = mSocialExchanges.Find(name);
	if (se)
	{
		return *se;
	}
	return nullptr;
}

void UCiFSocialExchangesLibrary::loadSocialGamesLibFromJson(const FString& jsonPath, const UObject* worldContextObject)
{
	TSharedPtr<FJsonObject> jsonObject;
	if (!UReadWriteFiles::readJson(jsonPath, jsonObject)) {
		return;
	}

	// iterate over the all the social games
	const auto socialGames = jsonObject->GetArrayField("SocialGamesLib");

	for (const auto sgJson : socialGames) {
		auto sg = UCiFSocialExchange::loadFromJson(sgJson->AsObject(), worldContextObject);
		mSocialExchanges.Add(sg->mName, sg);
	}
}
