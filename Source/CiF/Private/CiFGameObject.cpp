// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFGameObject.h"

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
	if (statusType < EStatus::FIRST_NOT_DIRECTED_STATUS) {
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
			}

			// setup the partner status if it has a partner and the status reciprocal - for now not sure about
			// which types are reciprocal, TODO maybe implement later
		}

		return;
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

		// TODO --	if this is a reciprocal status, like dating, i think it is also
		//			needed to call towards->addStatus(statusType, duration, this)
		//			but this is risky because we can get into a infinite loop - so maybe i need to check first
		//			if the other side already has the status and the addStatus call should happen in the
		//			end of the function so the other call would terminate because it will see it was already added here
		//			but other than that, i don't know why this is needed when we have
		//			social networks, where this data resides there, why it is also
		//			needed to be represented in statuses?

		if (mStatuses.Contains(statusType)) {
			mStatuses.Find(statusType)->statusArray.Add(newStatus);
		}
		else {
			FStatusArrayWrapper statusArrayWrapper;
			statusArrayWrapper.statusArray.Add(newStatus);
			mStatuses.Add(statusType, statusArrayWrapper);
		}
	}
}

void UCiFGameObject::removeStatus(const EStatus statusType, const FName towards)
{
	auto statusArrWrapper = mStatuses.Find(statusType);
	if (statusArrWrapper) {
		for (int32 i = statusArrWrapper->statusArray.Num() - 1; i >= 0; i--) {
			if (statusArrWrapper->statusArray[i]->mDirectedTowards == towards) {
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
	for (auto it = mStatuses.CreateIterator(); it; ++it) {
		// loop backwards through the array to remove status to not mess with indices while passing over the array
		for (int32 i = it.Value().statusArray.Num() - 1; i >= 0; i--) {
			if (it.Value().statusArray[i]->updateRemainingDuration(timeElapsed) <= 0) {
				removeStatus(it.Key(), it.Value().statusArray[i]->mDirectedTowards);
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
	mObjectName = FName(json->GetStringField("_name"));
	json->TryGetNumberField("_networkID", mNetworkId);

	const TArray<TSharedPtr<FJsonValue>>* traitsJson;
	if (json->TryGetArrayField("Trait", traitsJson)) {
		for (const auto traitJson : *traitsJson) {
			const auto traitEnum = StaticEnum<ETrait>();
			mTraits.Add(static_cast<ETrait>(traitEnum->GetValueByName(FName(traitJson->AsString()))));
		}	
	}
	
	// const auto traitsJson = json->GetArrayField("Trait");
	// for (const auto traitJson : traitsJson) {
	// 	const auto traitEnum = StaticEnum<ETrait>();
	// 	mTraits.Add(static_cast<ETrait>(traitEnum->GetValueByName(FName(traitJson->AsString()))));
	// }

	const TArray<TSharedPtr<FJsonValue>>* statusesJson;
	if (json->TryGetArrayField("Status", statusesJson)) {
		for (const auto statusJson : *statusesJson) {
			const auto statusEnum = StaticEnum<EStatus>();
			const auto statusType = static_cast<EStatus>(statusEnum->
				GetValueByName(FName(statusJson->AsObject()->GetStringField("_type"))));
			const FName towardsName(statusJson->AsObject()->GetStringField("_to"));
			addStatus(statusType, 0, towardsName); // TODO - why the status in the json doesn't have duration?
		}
	}
	
	// const auto statusesJson = json->GetArrayField("Status");
	// for (const auto statusJson : statusesJson) {
	// 	const auto statusEnum = StaticEnum<EStatus>();
	// 	const auto statusType = static_cast<EStatus>(statusEnum->
	// 		GetValueByName(FName(statusJson->AsObject()->GetStringField("_type"))));
	// 	const FName towardsName(statusJson->AsObject()->GetStringField("_to"));
	// 	addStatus(statusType, 0, towardsName); // TODO - why the status in the json doesn't have duration?
	// }
}


// Called when the game starts
void UCiFGameObject::BeginPlay()
{
	Super::BeginPlay();

	// ...
}


// Called every frame
void UCiFGameObject::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}
