// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFGameObjectStatus.h"
#include "CiFGameObject.h"
#include "CiFManager.h"
#include "CiFSubsystem.h"

TMap<EStatus, FStatusTypesArrayWrapper> UCiFGameObjectStatus::mStatusCategories = UCiFGameObjectStatus::initializeStatusCategoriesMap();

void UCiFGameObjectStatus::setDuration(const int32 newDuration)
{
	if (mHasDuration) {
		mInitialDuration = newDuration;
		mRemainingDuration = newDuration;
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Trying to set duration to a status with hasDuration set to false"));
	}
}

int32 UCiFGameObjectStatus::updateRemainingDuration(const int32 timeElapsed)
{
	mRemainingDuration -= timeElapsed;
	return mRemainingDuration;
}

void UCiFGameObjectStatus::init(const EStatus type, const int32 initialDuration, const FName towards)
{
	mType = type;
	mHasDuration = false;
	mBinary = false;

	if (initialDuration > 0) {
		mInitialDuration = initialDuration;
		mRemainingDuration = initialDuration;
		mHasDuration = true;
	}

	if (towards != "") {
		mDirectedTowards = towards;
		mBinary = true;
	}
}

TMap<EStatus, FStatusTypesArrayWrapper> UCiFGameObjectStatus::initializeStatusCategoriesMap()
{
	TMap<EStatus, FStatusTypesArrayWrapper> outMap;

	// TODO: note to use add() to add elements instead of operator []. maybe in static method there is a problem with that

	outMap.Add(EStatus::CAT_FEELING_BAD,
	           FStatusTypesArrayWrapper{
		           .mStatusTypes = {
			           EStatus::EMBARRASSED, EStatus::SHAKEN, EStatus::SAD, EStatus::ANXIOUS, EStatus::GUILTY, EStatus::CONFUSED
		           }
	           });

	outMap.Add(EStatus::CAT_FEELING_GOOD,
	           FStatusTypesArrayWrapper{
		           .mStatusTypes = {
			           EStatus::EXCITED, EStatus::CHEERFUL
		           }
	           });

	outMap.Add(EStatus::CAT_FEELING_BAD_ABOUT_SOMEONE,
	           FStatusTypesArrayWrapper{
		           .mStatusTypes = {EStatus::ANGRY_AT, EStatus::ENVIES, EStatus::AFRAID_OF}
	           });

	outMap.Add(EStatus::CAT_FEELING_GOOD_ABOUT_SOMEONE,
	           FStatusTypesArrayWrapper{
		           .mStatusTypes = {EStatus::GRATEFUL_TOWARD}
	           });

	outMap.Add(EStatus::CAT_REPUTATION_BAD,
	           FStatusTypesArrayWrapper{.mStatusTypes = {}});

	outMap.Add(EStatus::CAT_HAS_DURATION,
	           FStatusTypesArrayWrapper{
		           .mStatusTypes = {EStatus::SHAKEN, EStatus::HEARTBROKEN, EStatus::DRUNK, EStatus::TIPSY, EStatus::OFFENDED}
	           });

	outMap.Add(EStatus::CAT_USABLE_ITEMS_STATUS,
	           FStatusTypesArrayWrapper{
		           .mStatusTypes = {EStatus::DRINKABLE, EStatus::EATABLE, EStatus::LOCKED, EStatus::UNLOCKED, EStatus::HIDDEN}
	           });

	outMap.Add(EStatus::CAT_URGE_STATUSES,
	           FStatusTypesArrayWrapper{
		           .mStatusTypes = {
			           EStatus::QUEST_TYPE_URGE, EStatus::CHARACTER_TYPE_URGE, EStatus::ITEM_TYPE_URGE, EStatus::KNOWLEDGE_TYPE_URGE
		           }
	           });

	return outMap;
}
