// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFSocialExchangeContext.h"
#include "CiFGameObject.h"
#include "CiFManager.h"
#include "CiFPredicate.h"
#include "CiFRule.h"
#include "CiFSocialExchangesLibrary.h"
#include "CiFSubsystem.h"

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
		if (labelType != ESFDBLabelType::WILDCARD) {
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
				if ((labelType == ESFDBLabelType::WILDCARD) ||
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
		if (labelType != ESFDBLabelType::WILDCARD) {
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
		if (!((labelType == ESFDBLabelType::WILDCARD) ||
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

UCiFRule* UCiFSocialExchangeContext::getChange() const
{
	const auto cifManager = GetWorld()->GetGameInstance()->GetSubsystem<UCiFSubsystem>()->getInstance();
	if (mIsBackstory) return NewObject<UCiFRule>(); //make empty rule and bail early if this is a backstory TODO: why?
	auto sg = cifManager->mSocialExchangesLib->getSocialExchangeByName(mGameName);
	if (sg) {
		return sg->getEffectById(mEffectId)->mChange;
	}
	UE_LOG(LogTemp, Error, TEXT("Couldn't find the social game %s"), *(mGameName.ToString()));
	return nullptr;
}

UCiFSocialExchangeContext* UCiFSocialExchangeContext::loadFromJson(const TSharedPtr<FJsonObject> json, const UObject* worldContextObject)
{
	const auto sgc = NewObject<UCiFSocialExchangeContext>(const_cast<UObject*>(worldContextObject));
	sgc->UCiFSFDBContext::loadFromJson(json, worldContextObject);

	sgc->mIsBackstory = sgc->mTime < 0;

	FString name;
	sgc->mInitiatorName = json->TryGetStringField(TEXT("_initiator"), name) ? FName(name) : "";
	sgc->mResponderName = json->TryGetStringField(TEXT("_responder"), name) ? FName(name) : "";
	sgc->mOtherName = json->TryGetStringField(TEXT("_other"), name) ? FName(name) : "";

	sgc->mInitiatorScore = 0;
	sgc->mResponderScore = 0;


	if (sgc->mIsBackstory && false) { // TODO - i use backstory of promweek characters while my chars are of mismanor
		sgc->mPerformanceRealization = json->TryGetStringField(TEXT("_PerformanceRealizationString"), name) ? FName(name) : "";
		auto sfdbLblEnum = StaticEnum<ESFDBLabelType>();
		sgc->mSFDBLabel = {
			.from = sgc->mInitiatorName, .to = sgc->mResponderName,
			.type = static_cast<ESFDBLabelType>(sfdbLblEnum->GetValueByName(FName(json->GetStringField(TEXT("_SFDBLabel")))))
		};
	}
	else {
		json->TryGetNumberField(TEXT("_initiatorScore"), sgc->mInitiatorScore);
		json->TryGetNumberField(TEXT("_responderScore"), sgc->mInitiatorScore);
		sgc->mChosenItemCKB = json->TryGetStringField(TEXT("_chosenItemCKB"), name) ? FName(name) : "";
		sgc->mEffectId = CIF_INVALID_ID;
		json->TryGetNumberField(TEXT("_effectID"), sgc->mEffectId);
		sgc->mGameName = json->TryGetStringField(TEXT("_gameName"), name) ? FName(name) : "";
	}

	// TODO - below are stuff that i'm not gonna implement yet because i'm not sure they are relevant.
	// maybe after i implement more of the code i will see where it is used.


	// sgc.referenceSFDB = context.@socialGameContextReference;
	// if (context.@label.toString()) sgc.label = context.@label;
	// if (context.@labelFrom.toString()) sgc.labelArg1 = context.@labelFrom;
	// if (context.@labelDirectedAt.toString()) sgc.labelArg2 = context.@labelDirectedAt;
	// 			
	// //get the ckb predicates if they exists
	// for each(var ckbPredXML:XML in context..Predicate) {
	// 	sgc.queryCKB=predicateParse(ckbPredXML);
	// }

	return sgc;
}
