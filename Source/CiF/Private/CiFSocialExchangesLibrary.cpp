// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFSocialExchangesLibrary.h"

#include "CiFManager.h"
#include "CiFSocialExchange.h"
#include "CiFSocialFactsDataBase.h"
#include "CiFSubsystem.h"
#include "CiFTrigger.h"
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

	const auto cifManager = GetWorld()->GetGameInstance()->GetSubsystem<UCiFSubsystem>()->getInstance();
	
	// iterate over the all the social games
	const auto socialGames = jsonObject->GetArrayField("SocialGamesLib");

	for (const auto sgJson : socialGames) {
		auto sg = UCiFSocialExchange::loadFromJson(sgJson->AsObject(), worldContextObject);

		if (sg->mName == "TriggerGame") {
			for (const auto e : sg->mEffects) {
				auto t = NewObject<UCiFTrigger>();
				t->mReferenceAsNLG = e->mReferenceAsNLG;
				t->mCondition = e->mCondition;
				t->mChange = e->mChange;
				cifManager->mSFDB->mTriggers.Add(t);
			}
		}
		else if (sg->mName == "StoryTriggerGame") {
			for (const auto e : sg->mEffects) {
				auto t = NewObject<UCiFTrigger>();
				t->mReferenceAsNLG = e->mReferenceAsNLG;
				t->mCondition = e->mCondition;
				t->mChange = e->mChange;
				cifManager->mSFDB->mStoryTriggers.Add(t);
			}
		}
		else {
			mSocialExchanges.Add(sg->mName, sg);
		}
	}
}
