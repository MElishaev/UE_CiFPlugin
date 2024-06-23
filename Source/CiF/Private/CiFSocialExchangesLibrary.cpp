// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFSocialExchangesLibrary.h"

#include "CiFSocialExchange.h"

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
