// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFSocialFactsDataBase.h"

#include "CiFManager.h"
#include "CiFPredicate.h"
#include "CiFRule.h"
#include "CiFSFDBContext.h"
#include "CiFSocialExchangeContext.h"
#include "CiFSubsystem.h"
#include "CiFTrigger.h"
#include "CiFTriggerContext.h"

TMap<ESFDBLabelType, FLabelCategoryArrayWrapper> UCiFSocialFactsDataBase::mSFDBLabelCategories = UCiFSocialFactsDataBase::initializeCategoriesMap(); 

int32 UCiFSocialFactsDataBase::getLowestContextTime() const
{
	return mContexts[0]->mTime;
}

int32 UCiFSocialFactsDataBase::getLatestContextTime() const
{
	return mContexts.Last()->mTime;
}

int UCiFSocialFactsDataBase::timeOfPredicateInHistory(const UCiFPredicate* pred,
                                                      const UCiFGameObject* x,
                                                      const UCiFGameObject* y,
                                                      const UCiFGameObject* z) const
{
	int latestTimeInSFDB = getLatestContextTime();
	const int window = (pred->mIsSFDB && pred->mWindowSize > 0 && pred->mSFDBOrder < 1)
		                   ? pred->mWindowSize
		                   : latestTimeInSFDB + 1;

	int32 i = mContexts.Num() - 1;
	while ((i >= 0) && mContexts[i]->mTime > latestTimeInSFDB - window) {
		if (mContexts[i]->isPredicateInChange(pred, x, y, z)) {
			return mContexts[i]->mTime;
		}
		i--;
	}

	return INVALID_TIME;
}

bool UCiFSocialFactsDataBase::isPredicateInHistory(const UCiFPredicate* pred,
                                                   const UCiFGameObject* x,
                                                   const UCiFGameObject* y,
                                                   const UCiFGameObject* z) const
{
	return timeOfPredicateInHistory(pred, x, y, z) != INVALID_TIME;
}

void UCiFSocialFactsDataBase::findLabelFromValues(TArray<int32>& outMatchingIndices,
                                                  const ESFDBLabelType label,
                                                  const UCiFGameObject* c1,
                                                  const UCiFGameObject* c2,
                                                  const UCiFGameObject* c3,
                                                  int window,
                                                  const UCiFPredicate* pred) const
{
	if (mContexts.IsEmpty()) {
		UE_LOG(LogTemp, Warning, TEXT("Contexts is empty"));
		return;
	}

	const int32 timeToStopSearch = (window <= 0) ? getLowestContextTime() - 1 : getLatestContextTime() - window;

	//NOTE: this assumes that all entries are in order such that the most recent action is last in contexts
	for (int i = 0; i < mContexts.Num(); i++) {
		if ((mContexts[i]->getType() == ESFDBContextType::SOCIAL_GAME) || (mContexts[i]->getType() == ESFDBContextType::TRIGGER) &&
			(mContexts[i]->mTime > timeToStopSearch)) {

			if (mContexts[i]->getType() == ESFDBContextType::SOCIAL_GAME) {
				const auto sgc = static_cast<UCiFSocialExchangeContext*>(mContexts[i]);
				if (pred && pred->mIsNumTimesUniquelyTruePred) {
					//Call a strict version of this. Which requires a from because it is numTimesUniquelyTrue
					if (sgc->doesSFDBLabelMatchStrict(label, c1, c2, c3, pred)) {
						outMatchingIndices.Add(sgc->mTime);
					}
				}
				else {
					if (sgc->doesSFDBLabelMatch(label, c1, c2, c3, pred)) {
						outMatchingIndices.Add(sgc->mTime);
					}
				}	
			}
			else if (mContexts[i]->getType() == ESFDBContextType::TRIGGER) {
				const auto tc = static_cast<UCiFTriggerContext*>(mContexts[i]);
				if (pred && pred->mIsNumTimesUniquelyTruePred) {
					if (tc->doesSFDBLabelMatchStrict(label, c1, c2, c3, pred)) {
						outMatchingIndices.Add(tc->mTime);
					}
				}
				else {
					if (tc->doesSFDBLabelMatch(label, c1, c2, c3, pred)) {
						outMatchingIndices.Add(tc->mTime);
					}
				}
			}
		}
	}
}

UCiFTrigger* UCiFSocialFactsDataBase::getTriggerByID(uint64_t id) const
{
	auto trigger = mTriggers.FindByPredicate([=](const UCiFTrigger* t) { return t->mId == id; });
	if (trigger) {
		return *trigger;
	}
	return nullptr;
}

bool UCiFSocialFactsDataBase::doesMatchLabelOrCategory(const ESFDBLabelType contextLabel, const ESFDBLabelType predicateLabel) noexcept
{
	if (predicateLabel <= ESFDBLabelType::CAT_LAST) {
		// the predicate label is category, see if any of the labels in the category matches @contextLabel
		for (const auto catLabel : mSFDBLabelCategories[predicateLabel].mCategoryLabels) {
			if (catLabel == contextLabel) {
				return true;
			}
		}
		return false;
	}

	return contextLabel == predicateLabel;
}

TMap<ESFDBLabelType, FLabelCategoryArrayWrapper> UCiFSocialFactsDataBase::initializeCategoriesMap()
{
	TMap<ESFDBLabelType, FLabelCategoryArrayWrapper> outMap;

	// TODO: note to use add() to add elements instead of operator []. maybe in static method there is a problem with that
	
	outMap.Add(ESFDBLabelType::CAT_NEGATIVE, FLabelCategoryArrayWrapper{
		.mCategoryLabels = {
			ESFDBLabelType::MEAN, ESFDBLabelType::TABOO, ESFDBLabelType::RUDE
		}
	});

	outMap.Add(ESFDBLabelType::CAT_POSITIVE, FLabelCategoryArrayWrapper{
		.mCategoryLabels = {
			ESFDBLabelType::COOL, ESFDBLabelType::FUNNY
		}
	});

	outMap.Add(ESFDBLabelType::CAT_FLIRT, FLabelCategoryArrayWrapper{
		.mCategoryLabels = {ESFDBLabelType::ROMANTIC, ESFDBLabelType::FAILED_ROMANCE}
	});
	
	return outMap;
}

void UCiFSocialFactsDataBase::addContext(UCiFSFDBContext* context)
{
	// todo - this is for now not optimized because we sort it on every addition. (i think this can be done with heapsort and heappush methods of array)
	// it would better be to store the context in a heap to be able to insert in O(logn) instead of O(nlogn)
	mContexts.Add(context);
	mContexts.Sort([](UCiFSFDBContext& c1, UCiFSFDBContext& c2) { return c1.mTime <= c2.mTime; });
}

void UCiFSocialFactsDataBase::runTriggers(TArray<UCiFGameObject*> cast)
{
	const auto cifManager = GetWorld()->GetGameInstance()->GetSubsystem<UCiFSubsystem>()->getInstance();

	TArray<UCiFGameObject*> potentialChars;
	if (!cast.IsEmpty()) {
		potentialChars = cast;
	}
	else {
		cifManager->getAllGameObjectsOfType(potentialChars, ECiFGameObjectType::CHARACTER);
	}


	TArray<UCiFTrigger*> triggersToApply;
	TArray<UCiFGameObject*> firstRoles;
	TArray<UCiFGameObject*> secondRoles;
	TArray<UCiFGameObject*> thirdRoles;
	
	// run each trigger on every duple of characters or triple where needed by trigger (only characters for now because the current
	// triggers involve only statuses between characters. later on items could also be added)
	// TODO - why not run all triggers only on the characters that participated in the last social move that this method was called after?
	
	for (auto trigger : mTriggers) {
		for (auto firstChar : potentialChars) {
			for (auto secondChar : potentialChars) {
				if (firstChar != secondChar) {
					if (trigger->isThirdCharacterRequired()) {
						for (auto thirdChar : potentialChars) {
							if (thirdChar != firstChar && thirdChar != secondChar) {
								if (trigger->evaluateCondition(firstChar, secondChar, thirdChar)) {
									// valuate the trigger
									triggersToApply.Add(trigger);
									firstRoles.Add(firstChar);
									secondRoles.Add(secondChar);
									thirdRoles.Add(thirdChar);
								}
							}
						}
					}
					else {
						if (trigger->evaluateCondition(firstChar, secondChar)) {
							triggersToApply.Add(trigger);
							firstRoles.Add(firstChar);
							secondRoles.Add(secondChar);
							thirdRoles.Add(nullptr);
						}
					}
				}
			}
		}
	}

	//now that we have collected all the the triggers and characters involved, valuate them all
	
	// aPredHasValuated will be used to keep track of whether or not a triggerContext should be created
	// because it may be the case that the status was already the case, and thus a trigger context should not be created

	bool isPredHasValuated = false;
	for (int i = 0; i < triggersToApply.Num(); i++) {
		isPredHasValuated = false;
		// go through each change predicate and treat status that are already the case different than those that aren't
		//this is all part of making sure that we don't contantly display "cheating" every turn while someone is dating two characters
		for (auto changePred : triggersToApply[i]->mChange->mPredicates) {
			//figure out who the predicate should be applied to
			UCiFGameObject* fromChar = nullptr;
			auto primaryVal = changePred->getRoleValue(changePred->mPrimary);
			if (primaryVal == "initiator") fromChar = firstRoles[i];
			if (primaryVal == "responder") fromChar = secondRoles[i];
			if (primaryVal == "other") fromChar = thirdRoles[i];
			else fromChar = cifManager->getGameObjectByName(primaryVal);

			UCiFGameObject* towardChar = nullptr;
			if (changePred->mType == EPredicateType::STATUS) {
				if (changePred->mStatusType >= EStatus::FIRST_DIRECTED_STATUS) {
					auto secondaryVal = changePred->getRoleValue(changePred->mSecondary);
					if (secondaryVal == "initiator") towardChar = firstRoles[i];
					if (secondaryVal == "responder") towardChar = secondRoles[i];
					if (secondaryVal == "other") towardChar = thirdRoles[i];
					else towardChar = cifManager->getGameObjectByName(secondaryVal);
				}

				if (fromChar && fromChar->getStatus(changePred->mStatusType, towardChar->mObjectName)) {
					//if we are here, then we know that the fromChar has the status
					if (changePred->mIsNegated) {
						//this deals with removing status, which warrants a new trigger context
						isPredHasValuated = true;
						changePred->valuation(firstRoles[i], secondRoles[i], thirdRoles[i]);
					}
					else {
						//this is the case where rather than apply the status, we only reset its remaining duration. This is the
						//case that we do not want to create a new trigger context for.
						fromChar->getStatus(changePred->mStatusType, towardChar->mObjectName)->mRemainingDuration = UCiFGameObjectStatus::DEFAULT_INITIAL_DURATION;
					}
				}
				else if (!changePred->mIsNegated) {
					//this is the "normal case" where we simple apply the change predicate
					isPredHasValuated = true;
					changePred->valuation(firstRoles[i], secondRoles[i], thirdRoles[i]);
				}
			}
			else {
				//this is the normal, non-status case
				isPredHasValuated = true;
				changePred->valuation(firstRoles[i], secondRoles[i], thirdRoles[i]);
			}
		}
		// make trigger context
		if (isPredHasValuated) {
			auto tc = triggersToApply[i]->makeTriggerContext(cifManager->mTime, firstRoles[i], secondRoles[i], thirdRoles[i]);
			addContext(tc);
		}
	}
}

UCiFSocialFactsDataBase* UCiFSocialFactsDataBase::loadFromJson(const TSharedPtr<FJsonObject> json, const UObject* worldContextObject)
{
	const auto sfdb = NewObject<UCiFSocialFactsDataBase>(const_cast<UObject*>(worldContextObject));

	// todo if you load sfdb from file, it is mainly for backstory, or in case you want to set some initial state of social
	// interactions that occured in the game 
	
	return sfdb;
}
