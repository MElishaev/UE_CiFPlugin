// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFSocialExchangeContext.h"
#include "CiFGameObject.h"
#include "CiFPredicate.h"

ESFDBContextType UCiFSocialExchangeContext::getType() const
{
	return ESFDBContextType::SOCIAL_GAME;
}

bool UCiFSocialExchangeContext::doesSFDBLabelMatchStrict(const ESFDBLabelType labelType,
                                                         const UCiFGameObject* first,
                                                         const UCiFGameObject* second,
                                                         const UCiFGameObject* third,
                                                         const UCiFPredicate* pred) const
{
	if (mIsBackstory) {
		// if not wildcard, labels must match
		if (labelType != ESFDBLabelType::LABEL_WILDCARD) {
			if (mSFDBLabel.type != labelType) {
				return false;
			}
		}

		// explanation in doc
		if (first && (first->mObjectName != mInitiatorName)) {
			return false;
		}

		// explanation in doc
		if (second && (second->mObjectName != mResponderName)) {
			return false;
		}

		return true;
	}
	else {
		if (!mSFDBLabels.IsEmpty()) {
			for (const auto& sfdbLabel : mSFDBLabels) {
				if ((labelType == ESFDBLabelType::LABEL_WILDCARD) ||
					UCiFSocialFactsDataBase::doesMatchLabelOrCategory(sfdbLabel.type, labelType)) {
					//in here either the label matches or it is a wild card. check characters
					if (sfdbLabel.from == first->mObjectName) {
						if (second) {
							if (second->mObjectName == sfdbLabel.to) {
								return true;
							}
						}
						else if (sfdbLabel.to == "") {
							return true;
						}
						// no first char was present here - move on to the next by doing nothing
					}
				}
			}
			return false;
		}
	}
	return false;
}

bool UCiFSocialExchangeContext::doesSFDBLabelMatch(const ESFDBLabelType labelType,
                                                   const UCiFGameObject* first,
                                                   const UCiFGameObject* second,
                                                   const UCiFGameObject* third,
                                                   const UCiFPredicate* pred) const
{
	if (mIsBackstory) {
		// if this context is a backstory context, the first and second character
		// parameters must match the context's initiator and responder respectively
		if (labelType != ESFDBLabelType::LABEL_WILDCARD) {
			if (mSFDBLabel.type != labelType) {
				return false;
			}
		}

		// if first is null treat as a wildcard
		//in backstory contexts, the first's name must always match the context's initiator if firstCharacter exists
		if (first && first->mObjectName != mInitiatorName) {
			return false;
		}

		//if secondCharacter is null or if the label is not directed, treat it as a wildcard.
		//Note: right now we only have directed labels
		if (second && second->mObjectName != mResponderName) {
			return false;
		}

		return true; // if we got here, we found a match
	}

	// we reach here if it isn't a backstory context
	for (const auto sfdbLabel : mSFDBLabels) {
		bool isTrue = true;
		if (!((labelType == ESFDBLabelType::LABEL_WILDCARD) ||
			UCiFSocialFactsDataBase::doesMatchLabelOrCategory(sfdbLabel.type, labelType))) {
			isTrue = false; // if the label doesn't match and it isn't a wildcard fail - todo - why?
		}

		if (first && (first->mObjectName != sfdbLabel.from)) {
			isTrue = false;
		}

		if (second && (second->mObjectName != sfdbLabel.to)) {
			isTrue = false;
		}

		if (isTrue) {
			return true;
		}
	}

	return false;
}
