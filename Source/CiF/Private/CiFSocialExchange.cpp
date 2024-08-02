// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFSocialExchange.h"

#include "CiFCast.h"
#include "CiFInstantiation.h"
#include "CiFEffect.h"
#include "CiFKnowledge.h"
#include "CiFInfluenceRule.h"
#include "CiFInfluenceRuleSet.h"
#include "CiFManager.h"
#include "CiFSubsystem.h"
#include "CiFItem.h"
#include "CiFRule.h"

void UCiFSocialExchange::addEffect(UCiFEffect* effect)
{
	// todo - in PW they implemented it by cloning the effect, why can't i use the one sent here?
	//			this could have repercussions if i misunderstand something of how it works in the general system
	mEffects.AddUnique(effect);
}

void UCiFSocialExchange::addInstantiation(UCiFInstantiation* instantiation)
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

UCiFInstantiation* UCiFSocialExchange::getInstantiationById(const uint32 id)
{
	auto inst = mInstantiations.FindByPredicate([=](UCiFInstantiation* i) { return i->mId == id; });
	if (inst) {
		return *inst;
	}
	return nullptr;
}

float UCiFSocialExchange::getInitiatorScore(UCiFCharacter* initiator,
                                            UCiFGameObject* responder,
                                            UCiFGameObject* other,
                                            UCiFSocialExchange* se,
                                            TArray<UCiFGameObject*> activeOtherCast)
{
	// if other cast was given use, else use all the cast in the game
	if (activeOtherCast.Num() > 0) {
		return mInitiatorIR->scoreRules(initiator, responder, other, se, activeOtherCast);
	}

	auto cifManager = GetWorld()->GetGameInstance()->GetSubsystem<UCiFSubsystem>()->getInstance();
	return mInitiatorIR->scoreRules(initiator, responder, other, se, TArray<UCiFGameObject*>(cifManager->mCast->mCharacters));
}

float UCiFSocialExchange::getResponderScore(UCiFCharacter* initiator,
                                            UCiFGameObject* responder,
                                            UCiFGameObject* other,
                                            UCiFSocialExchange* se,
                                            TArray<UCiFGameObject*> activeOtherCast)
{
	// if other cast was given use, else use all the cast in the game
	if (activeOtherCast.Num() > 0) {
		return mResponderIR->scoreRules(initiator, responder, other, se, activeOtherCast);
	}

	auto cifManager = GetWorld()->GetGameInstance()->GetSubsystem<UCiFSubsystem>()->getInstance();
	return mResponderIR->scoreRules(initiator, responder, other, se, TArray<UCiFGameObject*>(cifManager->mCast->mCharacters));
}

float UCiFSocialExchange::scoreSocialExchange(UCiFCharacter* initiator,
                                              UCiFGameObject* responder,
                                              TArray<UCiFGameObject*> activeOtherCast,
                                              bool isResponder)
{
	float totalScore = 0;
	auto cifManager = GetWorld()->GetGameInstance()->GetSubsystem<UCiFSubsystem>()->getInstance();
	auto possibleOthers = activeOtherCast.IsEmpty() ? TArray<UCiFGameObject*>(cifManager->mCast->mCharacters) : activeOtherCast;
	auto influenceRuleSet = isResponder ? mResponderIR : mInitiatorIR;

	// TODO - assumption for only 1 precondition - don't know why but will go with it for now
	if (!mPreconditions.IsEmpty()) {
		if (mPreconditions[0]->isThirdCharacterRequired()) {
			//if the precondition involves an other run the IRS for all others with
			//a static other (for others that satisfy the SE's preconditions)
			for (auto other : possibleOthers) {
				if ((other->mObjectName != initiator->mObjectName) && (other->mObjectName != responder->mObjectName) && (other->mGameObjectType == mOtherType)) {
					if (mPreconditions[0]->evaluate(initiator, responder, other, this)) {
						totalScore += influenceRuleSet->scoreRules(initiator,
						                                           responder,
						                                           other,
						                                           this,
						                                           possibleOthers,
						                                           "",
						                                           isResponder);
					}
				}
			}
		}
		else {
			// if there is a precondition, but it doesn't require a third, just score the IRS once with variable other
			if (mPreconditions[0]->evaluate(initiator, responder, nullptr, this)) {
				totalScore += influenceRuleSet->scoreRulesWithVariableOther(initiator,
				                                                            responder,
				                                                            nullptr,
				                                                            this,
				                                                            possibleOthers,
				                                                            "",
				                                                            isResponder);
			}
		}
	}
	else {
		//if there are no precondition, just score the IRS, once with a variable other
		totalScore += influenceRuleSet->scoreRulesWithVariableOther(initiator,
		                                                            responder,
		                                                            nullptr,
		                                                            this,
		                                                            possibleOthers,
		                                                            "",
		                                                            isResponder);
	}

	return totalScore;
}

bool UCiFSocialExchange::checkPreconditionsVariableOther(UCiFCharacter* initiator,
                                                         UCiFGameObject* responder,
                                                         TArray<UCiFGameObject*> activeOtherCast)
{
	if (mPreconditions.IsEmpty()) {
		return true; // no preconditions means it is automatically true
	}

	auto cifManager = GetWorld()->GetGameInstance()->GetSubsystem<UCiFSubsystem>()->getInstance();
	auto possibleOthers = activeOtherCast.IsEmpty() ? TArray<UCiFGameObject*>(cifManager->mCast->mCharacters) : activeOtherCast;

	bool requiresOther = false;
	for (const auto precond : mPreconditions) {
		if (precond->isThirdCharacterRequired()) {
			requiresOther = true;
		}
	}

	if (requiresOther) {
		for (const auto other : possibleOthers) {
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
			}
		}
	}
	else {
		for (const auto precond : mPreconditions) {
			if (!precond->evaluate(initiator, responder, nullptr, this)) {
				return false;
			}
		}
		return true;
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
		if (precond->isThirdCharacterRequired()) return true;
	}

	for (const auto ir : mInitiatorIR->mInfluenceRules) {
		if (ir->isThirdCharacterRequired()) return true;
	}

	for (const auto ir : mResponderIR->mInfluenceRules) {
		if (ir->isThirdCharacterRequired()) return true;
	}

	for (const auto e : mEffects) {
		if (e->mCondition->isThirdCharacterRequired()) return true;
		if (e->mChange->isThirdCharacterRequired()) return true;
	}

	return false;
}

bool UCiFSocialExchange::isThirdForSocialExchangePlay()
{
	for (const auto e : mEffects) {
		if (e->mCondition->isThirdCharacterRequired() || e->mChange->isThirdCharacterRequired())
			return true;
	}
	return false;
}

void UCiFSocialExchange::updateRequiresOther()
{
	for (const auto precond : mPreconditions) {
		mIsRequiresOther = mIsRequiresOther || precond->isThirdCharacterRequired();
	}
}

void UCiFSocialExchange::getPossibleOthers(TArray<UCiFGameObject*>& outOthers, const FName initiatorName, const FName responderName)
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
		UE_LOG(LogTemp, Log, TEXT("Didn't find any others"));
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("Found others"));
	}
}

UCiFSocialExchange* UCiFSocialExchange::loadFromJson(const TSharedPtr<FJsonObject> sgJson, const UObject* worldContextObject)
{
	auto sg = NewObject<UCiFSocialExchange>(const_cast<UObject*>(worldContextObject));
	
	sg->mName = FName(sgJson->GetStringField("_name"));
	UE_LOG(LogTemp, Log, TEXT("Parsing social game: %s"), *(sg->mName.ToString()));
	sg->mIsRequiresOther = sgJson->GetBoolField("_requiresOther");
	sg->mResponderType = static_cast<ECiFGameObjectType>(sgJson->GetIntegerField("_type"));
	sg->mOtherType = static_cast<ECiFGameObjectType>(sgJson->GetIntegerField("_othertype"));

	// populate intents array (will consist only 1 rule with 1 predicate within the rule - that's how it is in the json file)
	auto intentJson = sgJson->GetArrayField("Intents");
	for (const auto ruleJson : intentJson) {
		auto rule = UCiFRule::loadFromJson(ruleJson->AsObject(), worldContextObject);
		sg->mIntents.Add(rule);
	}

	// load preconditions
	const auto preconditionsJson = sgJson->GetArrayField("Preconditions");
	for (auto precondJson : preconditionsJson) {
		auto precond = UCiFRule::loadFromJson(precondJson->AsObject(), worldContextObject);
		sg->mPreconditions.Add(precond);
	}

	// load influence rules
	sg->mInitiatorIR = NewObject<UCiFInfluenceRuleSet>();
	const auto initiatorIrJson = sgJson->GetArrayField("InitiatorInfluenceRuleSet");
	for (auto irJson : initiatorIrJson) {
		auto ir = UCiFInfluenceRule::loadFromJson(irJson->AsObject(), worldContextObject);
		sg->mInitiatorIR->mInfluenceRules.Add(ir);
	}

	sg->mResponderIR = NewObject<UCiFInfluenceRuleSet>();
	const auto responderIrJson = sgJson->GetArrayField("ResponderInfluenceRuleSet");
	for (auto irJson : responderIrJson) {
		auto ir = UCiFInfluenceRule::loadFromJson(irJson->AsObject(), worldContextObject);
		sg->mResponderIR->mInfluenceRules.Add(ir);
	}

	// load effects
	const auto effectsJson = sgJson->GetArrayField("Effects");
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
