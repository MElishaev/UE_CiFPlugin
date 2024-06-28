// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFInstantiation.h"

UniqueIDGenerator UCiFInstantiation::mIDGenerator = UniqueIDGenerator(); 

UCiFInstantiation::UCiFInstantiation()
{
	mId = mIDGenerator.getId();
}

bool UCiFInstantiation::requiresOtherToPerform() const
{
	// TODO - this is based on the type of game you create with CiF.
	//			In case of Mismanor, this is based on the line of dialogue variables to see if they require other,
	//			because in mismanor the instantiation is dialogue
	return false; // this for now is false
}
