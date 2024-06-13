// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFGameObjectStatus.h"

void UCiFGameObjectStatus::setDuration(const int32 newDuration)
{
	if (mHasDuration)
	{
		mInitialDuration = newDuration;
		mRemainingDuration = newDuration;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Trying to set duration to a status with hasDuration set to false"));
	}
}

int32 UCiFGameObjectStatus::updateRemainingDuration(const int32 timeElapsed)
{
	mRemainingDuration -= timeElapsed;
	return mRemainingDuration;
}

void UCiFGameObjectStatus::init(const EStatus type, const int32 initialDuration, UCiFGameObject* towards)
{
	mType = type;
	mHasDuration = false;
	mBinary = false;
	
	if (initialDuration > 0)
	{
		mInitialDuration = initialDuration;
		mRemainingDuration = initialDuration;
		mHasDuration = true;
	}

	if (towards)
	{
		mDirectedTowards = towards;
		mBinary = true;		
	}
}
