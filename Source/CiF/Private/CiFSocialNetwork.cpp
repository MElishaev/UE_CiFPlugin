// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFSocialNetwork.h"

void UCiFSocialNetwork::init(const ESocialNetworkType networkType, const uint8 numOfCharacters, const uint8 maxVal)
{
	mMaxVal = maxVal;
	mType = networkType;

	mNetwork.SetNum(numOfCharacters);
	for (auto row : mNetwork)
	{
		row.SetNum(numOfCharacters);
	}
	setAllArrayElements(maxVal / 2);
	
}

void UCiFSocialNetwork::setWeight(const uint8 c1, const uint8 c2, const uint8 w)
{
	mNetwork[c1][c2] = w;
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
