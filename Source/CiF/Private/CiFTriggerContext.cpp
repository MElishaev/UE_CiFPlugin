// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFTriggerContext.h"

#include "CiFGameObject.h"
#include "CiFManager.h"
#include "CiFPredicate.h"
#include "CiFRule.h"
#include "CiFSocialFactsDataBase.h"
#include "CiFSubsystem.h"
#include "CiFTrigger.h"
#include "Kismet/GameplayStatics.h"

UniqueIDGenerator UCiFTriggerContext::mIDGenerator = UniqueIDGenerator();

UCiFTriggerContext::UCiFTriggerContext()
{
	mId = mIDGenerator.getId();
}

ESFDBContextType UCiFTriggerContext::getType() const
{
	return ESFDBContextType::TRIGGER;
}

bool UCiFTriggerContext::isPredicateInChange(const UCiFPredicate* pred,
                                             const UCiFGameObject* x,
                                             const UCiFGameObject* y,
                                             const UCiFGameObject* z)
{
	const auto changeRule = getChange(); // get ref to the social game this context is about
	for (const auto predInChange : changeRule->mPredicates) {
		// see if the predicate matches 
		if (UCiFPredicate::equalsValuationStructure(pred, predInChange)) {
			// see if the roles of the characters match
			if (doPredicateRoleMatchCharacterVariables(predInChange, const_cast<UCiFGameObject*>(x),
				const_cast<UCiFGameObject*>(y), const_cast<UCiFGameObject*>(z), const_cast<UCiFPredicate*>(pred))) {
				return true;
			}
		}
	}

	return false;
}

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

UCiFRule* UCiFTriggerContext::getChange() const
{
	if (mId == UCiFTrigger::mStatusTimeoutTriggerID) {
		return mStatusTimeoutChange;
	}
	const auto cifManager = GetWorld()->GetGameInstance()->GetSubsystem<UCiFSubsystem>()->getInstance();
	const auto trigger = cifManager->mSFDB->getTriggerByID(mId);
	return trigger->mChange;
}

bool UCiFTriggerContext::doPredicateRoleMatchCharacterVariables(UCiFPredicate* predInChange,
                                                                UCiFGameObject* c1,
                                                                UCiFGameObject* c2,
                                                                UCiFGameObject* c3,
                                                                UCiFPredicate* predInEvalRule) const
{
	/*The trick to this function is that the x,y, and z are in correspondence with the predicates primary
	 * secondary, and tertiary character variables. This means we need to translate the predicates character
	 * variables to actual character names (if they are roles and not direct character names inititally) via
	 * the name<=>role mapping in the this SocialGameContext.
	 */
	if ((c1 != nullptr) != (predInChange->mPrimary != "")) return false;   // checks if only one of them exists
	if ((c2 != nullptr) != (predInChange->mSecondary != "")) return false; // checks if only one of them exists

	//z is a case we want to ignore as it is likely to be in the context but no in the evaluation.
	//We want to be lax in this case
	if (predInChange->mType == EPredicateType::RELATIONSHIP) {
		bool res = false;
		// case where the ordering matches
		res = doesPredicateRoleMatch(predInEvalRule, predInEvalRule->mPrimary, predInChange, predInChange->mPrimary, c1, c2, c3) &&
			doesPredicateRoleMatch(predInEvalRule, predInEvalRule->mSecondary, predInChange, predInChange->mSecondary, c1, c2, c3);
		if (res) {
			return true;
		}

		// the other side of the bi-directional relationship is also true
		FName temp = predInChange->mPrimary;
		predInChange->mPrimary = predInChange->mSecondary;
		predInChange->mSecondary = temp;
		res = doesPredicateRoleMatch(predInEvalRule, predInEvalRule->mPrimary, predInChange, predInChange->mPrimary, c1, c2, c3) &&
			doesPredicateRoleMatch(predInEvalRule, predInEvalRule->mSecondary, predInChange, predInChange->mSecondary, c1, c2, c3);
		temp = predInChange->mPrimary;
		predInChange->mPrimary = predInChange->mSecondary;
		predInChange->mSecondary = temp;

		if (res) {
			return true;
		}
		return false;
	}

	if (!doesPredicateRoleMatch(predInEvalRule, predInEvalRule->mPrimary, predInChange, predInChange->mPrimary, c1, c2, c3) ||
		!doesPredicateRoleMatch(predInEvalRule, predInEvalRule->mSecondary, predInChange, predInChange->mSecondary, c1, c2, c3) ||
		!doesPredicateRoleMatch(predInEvalRule, predInEvalRule->mTertiary, predInChange, predInChange->mTertiary, c1, c2, c3)) {
		return false;
	}
	return true;
}

bool UCiFTriggerContext::doesPredicateRoleMatch(UCiFPredicate* predInEvalRule,
                                                const FName roleInEval,
                                                UCiFPredicate* predInChange,
                                                const FName roleInChange,
                                                UCiFGameObject* x,
                                                UCiFGameObject* y,
                                                UCiFGameObject* z) const
{
	FName characterReferredToInEvalRule;
	FName characterReferredToInPredInChange;

	// get the character referred to in pred.primary
	auto roleVal = predInEvalRule->getRoleValue(roleInEval);
	if (roleVal == "initiator" || roleVal == "x") {
		characterReferredToInEvalRule = x->mObjectName;
	}
	else if (roleVal == "responder" || roleVal == "y") {
		characterReferredToInEvalRule = y->mObjectName;
	}
	else if (roleVal == "other" || roleVal == "z") {
		characterReferredToInEvalRule = z->mObjectName;
	}
	else if (roleVal == "") {
		characterReferredToInEvalRule = "";
	}
	else {
		//it's a character name
		characterReferredToInEvalRule = roleInEval;
	}

	roleVal = predInChange->getRoleValue(roleInChange);
	if (roleVal == "initiator" || roleVal == "x") {
		characterReferredToInPredInChange = x->mObjectName;
	}
	else if (roleVal == "responder" || roleVal == "y") {
		characterReferredToInPredInChange = y->mObjectName;
	}
	else if (roleVal == "other" || roleVal == "z") {
		characterReferredToInPredInChange = z->mObjectName;
	}
	else if (roleVal == "") {
		characterReferredToInPredInChange = "";
	}
	else {
		//it's a character name
		characterReferredToInPredInChange = roleInChange;
	}

	return characterReferredToInEvalRule == characterReferredToInPredInChange;
}

UCiFTriggerContext* UCiFTriggerContext::loadFromJson(const TSharedPtr<FJsonObject> json, const UObject* worldContextObject)
{
	const auto tc = NewObject<UCiFTriggerContext>(const_cast<UObject*>(worldContextObject));

	tc->UCiFSFDBContext::loadFromJson(json, worldContextObject);

	FString name;
	tc->mInitiatorName = json->TryGetStringField("_initiator", name) ? FName(name) : ""; 
	tc->mResponderName = json->TryGetStringField("_responder", name) ? FName(name) : ""; 
	tc->mOtherName = json->TryGetStringField("_other", name) ? FName(name) : "";

	const auto ruleJson = json->GetObjectField("Rule");
	tc->mStatusTimeoutChange = UCiFRule::loadFromJson(ruleJson, worldContextObject);
	
	return tc;
}
