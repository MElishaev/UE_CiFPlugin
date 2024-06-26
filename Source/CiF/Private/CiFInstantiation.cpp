// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFInstantiation.h"

UniqueIDGenerator UCiFInstantiation::mIDGenerator = UniqueIDGenerator(); 

UCiFInstantiation::UCiFInstantiation()
{
	mId = mIDGenerator.getId();
}
