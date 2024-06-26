// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFSocialFactsDataBase.h"
#include "CiFPredicate.h"
#include "CiFSFDBContext.h"
#include "CiFSocialExchangeContext.h"
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
			ESFDBLabelType::LABEL_MEAN, ESFDBLabelType::LABEL_TABOO, ESFDBLabelType::LABEL_RUDE
		}
	});

	outMap.Add(ESFDBLabelType::CAT_POSITIVE, FLabelCategoryArrayWrapper{
		.mCategoryLabels = {
			ESFDBLabelType::LABEL_COOL, ESFDBLabelType::LABEL_FUNNY
		}
	});

	outMap.Add(ESFDBLabelType::CAT_FLIRT, FLabelCategoryArrayWrapper{
		.mCategoryLabels = {ESFDBLabelType::LABEL_ROMANTIC, ESFDBLabelType::LABEL_FAILED_ROMANCE}
	});
	
	return outMap;
}
