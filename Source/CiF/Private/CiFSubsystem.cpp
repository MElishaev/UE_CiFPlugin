// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFSubsystem.h"
#include "CiFManager.h"

UCiFManager* UCiFSubsystem::getInstance()
{
	if (mCiFInstance) {
		return mCiFInstance;
	}
	
	mCiFInstance = NewObject<UCiFManager>();
	return mCiFInstance;
}

UCifImplementationBase* UCiFSubsystem::getImplementation() const
{
	if (!mCiFInstance) {
		UE_LOG(LogTemp, Error, TEXT("CiF instance wasn't created. Call getInstance() before trying to retreive implementation"));
		return nullptr;
	}
	
	if (!mImpl) {
		UE_LOG(LogTemp, Error, TEXT("Implementation isn't set"));
	}
	return mImpl;
}

void UCiFSubsystem::setImplementation(UCifImplementationBase* impl)
{
	mImpl = impl;
	mImpl->mCifManager = mCiFInstance;
}
