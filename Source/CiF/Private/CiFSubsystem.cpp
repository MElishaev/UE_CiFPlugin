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
