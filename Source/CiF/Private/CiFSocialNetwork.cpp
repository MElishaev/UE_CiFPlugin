// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFSocialNetwork.h"
#include "CiFManager.h"
#include "CiFSubsystem.h"

void UCiFSocialNetwork::init(const ESocialNetworkType networkType, const uint8 numOfCharacters, const uint8 maxVal)
{
	mMaxVal = maxVal;
	mType = networkType;

	mNetwork.SetNum(numOfCharacters);
	for (int32 i = 0; i < numOfCharacters; i++) {
		mNetwork[i].SetNum(numOfCharacters);
	}
	setAllArrayElements(maxVal / 2);
}

void UCiFSocialNetwork::setWeight(const int32 c1, const int32 c2, const uint8 w)
{
	if (c1 < mNetwork.Num() && c2 < mNetwork.Num()) {
		mNetwork[c1][c2] = w;
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("Trying set weight to [%d][%d] while number of characters is %d"), c1, c2, mNetwork.Num());
	}
}

void UCiFSocialNetwork::addWeight(const uint8 c1, const uint8 c2, const int addition)
{
	mNetwork[c1][c2] = (mNetwork[c1][c2] + addition) <= mMaxVal ? mNetwork[c1][c2] + addition : mMaxVal;
}

void UCiFSocialNetwork::multiplyWeight(const uint8 c1, const uint8 c2, const float multiplier)
{
	mNetwork[c1][c2] = (mNetwork[c1][c2] * multiplier) <= mMaxVal ? mNetwork[c1][c2] * multiplier : mMaxVal;
}

uint8 UCiFSocialNetwork::getWeight(const uint8 c1, const uint8 c2)
{
	return mNetwork[c1][c2];
}

float UCiFSocialNetwork::getAverageOpinion(const uint8 c)
{
	float total = 0;
	for (size_t i = 0; i < mNetwork.Num(); i++)
	{
		if (i != c)
		{
			total += mNetwork[i][c];
		}
	}
	return total / (mNetwork.Num() - 1);
}

TArray<uint8> UCiFSocialNetwork::getRelationshipsAboveThreshold(const uint8 c, const uint8 th)
{
	TArray<uint8> idsArr;
	for (size_t i = 0; i < mNetwork.Num(); i++)
	{
		if ((c != i) && (mNetwork[c][i] > th))
		{
			idsArr.Add(i);
		}
	}
	return idsArr;
}

TArray<uint8> UCiFSocialNetwork::getReverseRelationshipsAboveThreshold(const uint8 c, const uint8 th)
{
	TArray<uint8> idsArr;
	for (size_t i = 0; i < mNetwork.Num(); i++)
	{
		if ((c != i) && (mNetwork[i][c] > th))
		{
			idsArr.Add(i);
		}
	}
	return idsArr;
}

UCiFSocialNetwork* UCiFSocialNetwork::loadFromJson(const TSharedPtr<FJsonObject> json, const UObject* worldContextObject)
{
	const auto sn = NewObject<UCiFSocialNetwork>(const_cast<UObject*>(worldContextObject));

	const auto snEnum = StaticEnum<ESocialNetworkType>();
	const auto snType = static_cast<ESocialNetworkType>(snEnum->GetValueByName(FName(json->GetStringField("_type"))));
	const auto numChars = json->GetNumberField("_numChars");
	const int8 maxVal = 100;
	
	sn->init(snType, numChars, maxVal);

	// now load the values from the json
	const auto cifManager = sn->GetWorld()->GetGameInstance()->GetSubsystem<UCiFSubsystem>()->getInstance();
	const auto edgesJson = json->GetArrayField("edge");
	for (const auto edgeJson : edgesJson) {
		const auto weight = edgeJson->AsObject()->GetNumberField("_value");
		const auto from = FName(edgeJson->AsObject()->GetStringField("_from"));
		const auto to = FName(edgeJson->AsObject()->GetStringField("_to"));
		const auto fromObject = cifManager->getGameObjectByName(from);
		const auto toObject = cifManager->getGameObjectByName(to);
		if (fromObject && toObject) {
			sn->setWeight(fromObject->mNetworkId, toObject->mNetworkId, weight);
		}
		else {
			UE_LOG(LogTemp, Error, TEXT("Couldn't find the characters that represented by this edge"));
		}
	}

	return sn;
}

void UCiFSocialNetwork::setAllArrayElements(uint8 val)
{
	for (auto& row : mNetwork)
	{
		for (auto& e : row)
		{
			e = val;
		}
	}
}
