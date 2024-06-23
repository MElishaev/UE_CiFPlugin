// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFTriggerContext.h"

#include "CiFGameObject.h"
#include "CiFSocialFactsDataBase.h"


bool UCiFTriggerContext::doesSFDBLabelMatchStrict(const ESFDBLabelType labelType,
                                                  const UCiFGameObject* first,
                                                  const UCiFGameObject* second,
                                                  const UCiFGameObject* third,
                                                  const UCiFPredicate* pred) const
{
	for (const auto sfdbLabel : mSFDBLabels) {
		if (labelType == ESFDBLabelType::LABEL_WILDCARD || UCiFSocialFactsDataBase::doesMatchLabelOrCategory(sfdbLabel.type, labelType)) {
			// either the label matches or it is a wildcard, check characters matching
			if (sfdbLabel.from == first->mObjectName) {
				if (second) {
					if (second->mObjectName == sfdbLabel.to) {
						return true;
					}
				}
				else if (sfdbLabel.to == "") {
					return true;
				}
			}
		}
	}
	return false;
}

bool UCiFTriggerContext::doesSFDBLabelMatch(const ESFDBLabelType labelType,
                                            const UCiFGameObject* first,
                                            const UCiFGameObject* second,
                                            const UCiFGameObject* third,
                                            const UCiFPredicate* pred) const
{
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
