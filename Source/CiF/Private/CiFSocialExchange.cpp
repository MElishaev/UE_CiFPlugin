// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFSocialExchange.h"
#include "Narrative/CifInstantiation.h"
#include "CiFEffect.h"
#include "CiFInfluenceRule.h"
#include "CiFInfluenceRuleSet.h"
#include "CiFManager.h"
#include "CiFSubsystem.h"
#include "CiFProspectiveMemory.h"
#include "CiFRule.h"

void UCiFSocialExchange::addEffect(UCiFEffect* effect)
{
	// todo - in PW they implemented it by cloning the effect, why can't i use the one sent here?
	//			this could have repercussions if i misunderstand something of how it works in the general system
	mEffects.AddUnique(effect);
}

void UCiFSocialExchange::addInstantiation(UCifInstantiation* instantiation)
{
	// todo - same comment as in @addEffect
	mInstantiations.AddUnique(instantiation);
}

UCiFEffect* UCiFSocialExchange::getEffectById(const uint32 id)
{
	auto effect = mEffects.FindByPredicate([=](UCiFEffect* e) { return e->mId == id; });
	if (effect) {
		return *effect;
	}
	return nullptr;
}

UCifInstantiation* UCiFSocialExchange::getInstantiationById(const uint32 id)
{
    // todo fix
	// auto inst = mInstantiations.FindByPredicate([=](UCifInstantiation* i) { return i->mId == id; });
	// if (inst) {
	// 	return *inst;
	// }
	return nullptr;
}

float UCiFSocialExchange::getInitiatorScore(UCiFCharacter* initiator,
                                            UCiFGameObject* responder,
                                            UCiFGameObject* other,
                                            UCiFSocialExchange* se)
{
	return mInitiatorIR->scoreRules(initiator, responder, other, se);
}

float UCiFSocialExchange::getResponderScore(UCiFCharacter* initiator,
                                            UCiFGameObject* responder,
                                            UCiFGameObject* other,
                                            UCiFSocialExchange* se)
{
	return mResponderIR->scoreRules(initiator, responder, other, se);
}

float UCiFSocialExchange::scoreSocialExchange(UCiFCharacter* initiator,
                                              UCiFGameObject* responder,
                                              UCiFGameObject*& bestOther,
                                              const TArray<UCiFGameObject*>& activeOtherCast,
                                              bool isResponder)
{
	/* the totalScore default value is 0. so if the rules for some other score less than 0,
	 * they will be discarded. this is kind of a weird behaviour, but anyway, others that score
	 * less than 0 we wouldn't want to play a social game including them */
	FScore_t totalScore = -100;
	const auto influenceRuleSet = isResponder ? mResponderIR : mInitiatorIR;
	bestOther = nullptr;

	if (mIsRequiresOther) {
		if (mOtherType != ECiFGameObjectType::CHARACTER) {
			UE_LOG(LogTemp, Verbose, TEXT("Social games with other type different than character isn't supported yet"));
			return totalScore;
		}
		for (auto other : activeOtherCast) {
			if ((other->mObjectName != initiator->mObjectName) && (other->mObjectName != responder->mObjectName) && (other->mGameObjectType == mOtherType)) {
				if (checkPreconditions(initiator, responder, other, this)) {
					UE_LOG(LogTemp, VeryVerbose, TEXT("scoring for other: %s"), *(other->mObjectName.ToString())); // todo - delete?
					const FScore_t localScore = influenceRuleSet->scoreRules(initiator,
																		  responder,
																		  other,
																		  this,
																		  "",
																		  isResponder);
					if (localScore >= totalScore) {
						// if the score is the same, just randomly pick between the 2 so the
						// behavior will be more dynamic
						if (totalScore == localScore) {
							bestOther = FMath::RandRange(0, 1) == 1 ? other : bestOther;
						}
						else {
							bestOther = other;
							totalScore = localScore;
						}
					}
				}
			}
		}
	}
	else {
		if (checkPreconditions(initiator, responder, nullptr, this)) {
			totalScore = influenceRuleSet->scoreRulesWithVariableOther(initiator,
			                                                           responder,
			                                                           nullptr,
			                                                           this,
			                                                           // possibleOthers,
			                                                           activeOtherCast,
			                                                           NAME_None,
			                                                           isResponder);
		}
	}
	
	return totalScore;
}

bool UCiFSocialExchange::checkPreconditionsVariableOther(UCiFCharacter* initiator,
                                                         UCiFGameObject* responder,
                                                         const TArray<UCiFGameObject*>& activeOtherCast)
{
	if (mPreconditions.IsEmpty()) {
		return true; // no preconditions means it is automatically true
	}

	if (mOtherType != ECiFGameObjectType::CHARACTER) {
		UE_LOG(LogTemp, Warning, TEXT("Social games with other type different than character isn't supported yet"));
		return false;
	}

	if (activeOtherCast.IsEmpty()) {
		for (const auto precond : mPreconditions) {
			if (!precond->evaluate(initiator, responder, nullptr, this)) {
				return false;
			}
		}
		return true;
	}
	else {
		for (const auto other : activeOtherCast) {
			bool isOtherSuitable = true;

			if ((other->mObjectName != initiator->mObjectName) && (other->mObjectName != responder->mObjectName)) {
				for (const auto precond : mPreconditions) {
					if (!precond->evaluate(initiator, responder, other, this)) {
						isOtherSuitable = false;
						break;
					}
				}
			}

			if (isOtherSuitable) {
				return true; // went over all precondition and none failed with the current other
				// todo - but why not return or flag this other or something? so it can be used in the scoring?
				//  it seems this method just tells if there is some other.. and then the scoring method will go over
				//  all the others again and will score... maybe there are more than 1 holding the preconditions
			}
		}
	}
	
	return false;
}

bool UCiFSocialExchange::checkPreconditions(UCiFCharacter* initiator,
                                            UCiFGameObject* responder,
                                            UCiFGameObject* other,
                                            UCiFSocialExchange* se)
{
	for (const auto precond : mPreconditions) {
		if (!precond->evaluate(initiator, responder, other, se)) {
			return false;
		}
	}
	return true;
}

bool UCiFSocialExchange::checkIntents(UCiFCharacter* initiator, UCiFGameObject* responder, UCiFGameObject* other, UCiFSocialExchange* se)
{
	for (const auto intent : mIntents) {
		if (!intent->evaluate(initiator, responder, other, se)) {
			return false;
		}
	}
	return true;
}

bool UCiFSocialExchange::isThirdNeededForIntentFormation()
{
	// checks in any of the members that can contain a third party if it is required
	
	for (const auto precond : mPreconditions) {
		if (precond->isRoleRequired("other")) return true;
	}

	for (const auto ir : mInitiatorIR->mInfluenceRules) {
		if (ir->isRoleRequired("other")) return true;
	}

	for (const auto ir : mResponderIR->mInfluenceRules) {
		if (ir->isRoleRequired("other")) return true;
	}

	for (const auto e : mEffects) {
		if (e->mCondition->isRoleRequired("other")) return true;
		if (e->mChange->isRoleRequired("other")) return true;
	}

	return false;
}

bool UCiFSocialExchange::isThirdForSocialExchangePlay()
{
	for (const auto e : mEffects) {
		if (e->mCondition->isRoleRequired("other") || e->mChange->isRoleRequired("other"))
			return true;
	}
	return false;
}

void UCiFSocialExchange::updateRequiresOther()
{
	for (const auto precond : mPreconditions) {
		mIsRequiresOther = mIsRequiresOther || precond->isRoleRequired("other");
	}
}

void UCiFSocialExchange::getPossibleOthers(TArray<UCiFGameObject*>& outOthers, const FName initiatorName, const FName responderName) const
{
	if (!mIsRequiresOther) return;
	const auto cifManager = GetWorld()->GetGameInstance()->GetSubsystem<UCiFSubsystem>()->getInstance();
	checkf(cifManager != nullptr, TEXT("Couldn't get cif manager"));
	
	TArray<UCiFGameObject*> possibleOthers = {};
	cifManager->getAllGameObjectsOfType(possibleOthers, mOtherType);

	const auto initiator = cifManager->getGameObjectByName(initiatorName);
	const auto responder = cifManager->getGameObjectByName(responderName);
	
	for (int32 j = 0; j < possibleOthers.Num(); j++) {
		UCiFGameObject* other = possibleOthers[j];
		if ((initiatorName != other->mObjectName) && (responderName != other->mObjectName)) {
			bool isOtherSuitable = true;
			// if other found not to hold the preconditions rules, move to the next
			for (int32 i = 0; i < mPreconditions.Num() && isOtherSuitable; i++) {
				if (!mPreconditions[i]->evaluate(initiator, responder, other, this)) {
					isOtherSuitable = false;
				}
			}

			// If a suitable other for preconditions is found, make sure the other ALSO is an appropriate type for the move
			// Should not be a problem for most moves (for example, RequestItem requires the other has trait "item")
			if (isOtherSuitable && !checkOtherType(other)) {
				isOtherSuitable = false;
			}
			if (isOtherSuitable) {
				outOthers.Add(other);
			}
		}
	}
	if (outOthers.IsEmpty()) {
		UE_LOG(LogTemp, Log, TEXT("Didn't find any others for %s"), *(mName.ToString()));
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("Found others for %s"), *(mName.ToString()));
	}
}

FCacheKey UCiFSocialExchange::getSocialExchangeExtendedIntentType() const
{
	// for now, intents of a SG have 1 rule in them with 1 predicate, so accessing it like this is ok.
	return mIntents[0]->mPredicates[0]->getExtendedIntentType(); 
}

UCiFSocialExchange* UCiFSocialExchange::loadFromJson(const TSharedPtr<FJsonObject> sgJson, const UObject* worldContextObject)
{
	auto sg = NewObject<UCiFSocialExchange>(const_cast<UObject*>(worldContextObject));
	
	sg->mName = FName(sgJson->GetStringField(TEXT("_name")));
	UE_LOG(LogTemp, VeryVerbose, TEXT("Parsing social game: %s"), *(sg->mName.ToString()));
	sg->mIsRequiresOther = sgJson->GetBoolField(TEXT("_requiresOther"));
	sg->mResponderType = static_cast<ECiFGameObjectType>(sgJson->GetIntegerField(TEXT("_type")));
	sg->mOtherType = static_cast<ECiFGameObjectType>(sgJson->GetIntegerField(TEXT("_othertype")));

	// populate intents array (will consist only 1 rule with 1 predicate within the rule - that's how it is in the json file)
	auto intentJson = sgJson->GetArrayField(TEXT("Intents"));
	for (const auto ruleJson : intentJson) {
		auto rule = UCiFRule::loadFromJson(ruleJson->AsObject(), worldContextObject);
		sg->mIntents.Add(rule);
	}

	// load preconditions
	const auto preconditionsJson = sgJson->GetArrayField(TEXT("Preconditions"));
	for (auto precondJson : preconditionsJson) {
		auto precond = UCiFRule::loadFromJson(precondJson->AsObject(), worldContextObject);
		sg->mPreconditions.Add(precond);
	}

	// load influence rules
	sg->mInitiatorIR = NewObject<UCiFInfluenceRuleSet>(const_cast<UObject*>(worldContextObject));
	const auto initiatorIrJson = sgJson->GetArrayField(TEXT("InitiatorInfluenceRuleSet"));
	for (auto irJson : initiatorIrJson) {
		auto ir = UCiFInfluenceRule::loadFromJson(irJson->AsObject(), worldContextObject);
		sg->mInitiatorIR->mInfluenceRules.Add(ir);
	}

	sg->mResponderIR = NewObject<UCiFInfluenceRuleSet>(const_cast<UObject*>(worldContextObject));
	const auto responderIrJson = sgJson->GetArrayField(TEXT("ResponderInfluenceRuleSet"));
	for (auto irJson : responderIrJson) {
		auto ir = UCiFInfluenceRule::loadFromJson(irJson->AsObject(), worldContextObject);
		sg->mResponderIR->mInfluenceRules.Add(ir);
	}

	// load effects
	const auto effectsJson = sgJson->GetArrayField(TEXT("Effects"));
	for (const auto effectJson : effectsJson) {
		sg->mEffects.Add(UCiFEffect::loadFromJson(effectJson->AsObject(), worldContextObject));		
	}

	// TODO - skip loading instantiations for now

	// TODO - before saving, sort all the predicates in all rules - for optimizations i assume

	sg->updateRequiresOther();

	return sg;
}

bool UCiFSocialExchange::checkOtherType(const UCiFGameObject* other) const
{
	if (other &&
		mOtherType == ECiFGameObjectType::CHARACTER ||
		mOtherType == ECiFGameObjectType::ITEM ||
		mOtherType == ECiFGameObjectType::KNOWLEDGE) {
		return true;
	}
	return false;
}
