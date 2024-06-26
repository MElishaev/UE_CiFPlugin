// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFEffect.h"

UniqueIDGenerator UCiFEffect::mIDGenerator = UniqueIDGenerator(); 

UCiFEffect::UCiFEffect()
{
	mId = mIDGenerator.getId();
}
