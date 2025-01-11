// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFGameObject.h"

#include "Dom/JsonObject.h"
#include "CiFGameObjectStatus.h"

// Sets default values for this component's properties
UCiFGameObject::UCiFGameObject()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

void UCiFGameObject::addTrait(const ETrait trait)
{
	mTraits.Add(trait);
}

bool UCiFGameObject::hasTrait(const ETrait trait) const
{
	return mTraits.Contains(trait);
}

bool UCiFGameObject::hasStatus(const EStatus statusType, const UCiFGameObject* towards) const
{
	const auto statusesWrapper = mStatuses.Find(statusType);
	if (statusesWrapper) {
		const auto statuses = statusesWrapper->statusArray;
		return statuses.ContainsByPredicate([=](const UCiFGameObjectStatus* status) {
			if (towards) {
				return (status->mType == statusType) && (towards->mObjectName == status->mDirectedTowards);
			}
			return status->mType == statusType;
		});
	}
	return false;
}

void UCiFGameObject::addStatus(const EStatus statusType, const int32 duration, const FName towards)
{
	// if the type of the status is category
	if (statusType < EStatus::LAST_CATEGORY_COUNT) {
		return addCategoryStatus(statusType, duration, towards);
	}

	// not a category status

	const auto status = getStatus(statusType, towards);
	if (status) {
		// if this object already has the status and also has duration, update the duration
		if (status->mHasDuration && duration > 0) {
			status->setDuration(duration);
		}
	}
	else {
		// create new status
		auto newStatus = NewObject<UCiFGameObjectStatus>();
		newStatus->init(statusType, duration, towards);

		if (mStatuses.Contains(statusType)) {
			mStatuses.Find(statusType)->statusArray.Add(newStatus);
		}
		else {
			FStatusArrayWrapper statusArrayWrapper;
			statusArrayWrapper.statusArray.Add(newStatus);
			mStatuses.Add(statusType, statusArrayWrapper);
			UE_LOG(LogTemp,
				   Log,
				   TEXT("Added status: %s %s"),
				   *(mObjectName.ToString()), *(newStatus->toString()));
		}
	}
}

void UCiFGameObject::removeStatus(const EStatus statusType, const FName towards)
{
	auto statusArrWrapper = mStatuses.Find(statusType);
	if (statusArrWrapper) {
		// loop backwards because removing elements while moving forwards messes with the indices of the array
		for (int32 i = statusArrWrapper->statusArray.Num() - 1; i >= 0; i--) {
			if (statusArrWrapper->statusArray[i]->mDirectedTowards == towards) {
				UE_LOG(LogTemp, Log, TEXT("%s removing status %s"), *(mObjectName.ToString()), *(statusArrWrapper->statusArray[i]->toString()));
				statusArrWrapper->statusArray.RemoveAt(i);
				break;
			}
		}

		if (statusArrWrapper->statusArray.IsEmpty()) {
			mStatuses.Remove(statusType);
		}
	}
}

void UCiFGameObject::updateStatusDurations(const int32 timeElapsed)
{
	for (const auto &[k, v] : mStatuses) {
		for (auto* status : v.statusArray) {
			if (status->mHasDuration && status->mRemainingDuration > 0) {
				status->mRemainingDuration--;
			}
		}
	}
}

UCiFGameObjectStatus* UCiFGameObject::getStatus(const EStatus statusType, const FName towards)
{
	auto statusesWrapper = mStatuses.Find(statusType);
	if (statusesWrapper) {
		auto statuses = statusesWrapper->statusArray;
		auto status = statuses.FindByPredicate([=](const UCiFGameObjectStatus* status) {
			return (status->mType == statusType) && (towards == status->mDirectedTowards);
		});
		if (status) {
			return *status;
		}
	}
	return nullptr;
}

void UCiFGameObject::loadFromJson(const TSharedPtr<FJsonObject> json, const UObject* worldContextObject)
{
	mObjectName = FName(json->GetStringField(TEXT("_name")));
	json->TryGetNumberField(TEXT("_networkID"), mNetworkId);

	const TArray<TSharedPtr<FJsonValue>>* traitsJson;
	if (json->TryGetArrayField(TEXT("Trait"), traitsJson)) {
		for (const auto traitJson : *traitsJson) {
			const auto traitEnum = StaticEnum<ETrait>();
			mTraits.Add(static_cast<ETrait>(traitEnum->GetValueByName(FName(traitJson->AsString()))));
		}	
	}
	
	// const auto traitsJson = json->GetArrayField(TEXT("Trait"));
	// for (const auto traitJson : traitsJson) {
	// 	const auto traitEnum = StaticEnum<ETrait>();
	// 	mTraits.Add(static_cast<ETrait>(traitEnum->GetValueByName(FName(traitJson->AsString()))));
	// }

	const TArray<TSharedPtr<FJsonValue>>* statusesJson;
	if (json->TryGetArrayField(TEXT("Status"), statusesJson)) {
		for (const auto statusJson : *statusesJson) {
			const auto statusEnum = StaticEnum<EStatus>();
			const auto statusType = static_cast<EStatus>(statusEnum->
				GetValueByName(FName(statusJson->AsObject()->GetStringField(TEXT("_type")))));
			FString to;
			if (statusJson->AsObject()->TryGetStringField(TEXT("_to"), to)) {
				addStatus(statusType, 0, FName(to)); // TODO - why the status in the json doesn't have duration?
			}
			else {
				addStatus(statusType); // TODO - why the status in the json doesn't have duration?
			}
		}
	}
	
	// const auto statusesJson = json->GetArrayField(TEXT("Status"));
	// for (const auto statusJson : statusesJson) {
	// 	const auto statusEnum = StaticEnum<EStatus>();
	// 	const auto statusType = static_cast<EStatus>(statusEnum->
	// 		GetValueByName(FName(statusJson->AsObject()->GetStringField(TEXT("_type")))));
	// 	const FName towardsName(statusJson->AsObject()->GetStringField(TEXT("_to")));
	// 	addStatus(statusType, 0, towardsName); // TODO - why the status in the json doesn't have duration?
	// }
}


// Called when the game starts
void UCiFGameObject::BeginPlay()
{
	Super::BeginPlay();

	// ...
}

void UCiFGameObject::addCategoryStatus(const EStatus statusType, const int32 duration, const FName towards)
{
	// apply all statuses in that category
	const auto statusCat = UCiFGameObjectStatus::mStatusCategories.Find(statusType);
	if (!statusCat) {
		UE_LOG(LogTemp, Error, TEXT("Didn't find status category %d"), statusType);	
		return;
	}
		
	for (const auto type : (*statusCat).mStatusTypes) {
		// see if character has status already
		const auto status = getStatus(type, towards);
		if (status) {
			continue;
		}

		// status not found, add it to character
		auto newStatus = NewObject<UCiFGameObjectStatus>();
		newStatus->init(statusType, duration, towards);
		if (mStatuses.Contains(statusType)) {
			mStatuses.Find(statusType)->statusArray.Add(newStatus);
		}
		else {
			FStatusArrayWrapper statusArrayWrapper;
			statusArrayWrapper.statusArray.Add(newStatus);
			mStatuses.Add(statusType, statusArrayWrapper);
			UE_LOG(LogTemp,
			   Log,
			   TEXT("Added status: %s %s"),
			   *(mObjectName.ToString()), *(newStatus->toString()));
		}
	}
}


// Called every frame
void UCiFGameObject::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}
