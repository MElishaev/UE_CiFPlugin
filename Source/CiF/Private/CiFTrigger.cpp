// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFTrigger.h"

#include "CiFRule.h"
#include "CiFTriggerContext.h"

ID_t UCiFTrigger::mStatusTimeoutTriggerID = -1;

UCiFTriggerContext* UCiFTrigger::makeTriggerContext(const int32 time, UCiFGameObject* x, UCiFGameObject* y, UCiFGameObject* z) const
{
	const auto tc = NewObject<UCiFTriggerContext>(GetOuter()); // outer acts as a world object context because this trigger was initialized with one
	
	tc->mTime = time;
	tc->mId = mId;
	tc->mInitiatorName = x->mObjectName;
	tc->mResponderName = (y) ? y->mObjectName : "";
	tc->mOtherName = (z) ? z->mObjectName: "";

	for (const auto p : mChange->mPredicates) {
		if (p->mType == EPredicateType::SFDB_LABEL) {
			auto to = p->getPrimaryCharacterNameFromVariables(x, y, z);
			auto from = p->getSecondaryCharacterNameFromVariables(x, y, z);
			auto type =  p->mSFDBLabel.type ;
			tc->mSFDBLabels.Emplace(from, to, type);
		}
	}
			
	return tc;
}

UCiFTrigger* UCiFTrigger::loadFromJson(const TSharedPtr<FJsonObject> json, const UObject* worldContextObject)
{
	auto t = NewObject<UCiFTrigger>(const_cast<UObject*>(worldContextObject));

	FString strContainer;
	t->mReferenceAsNLG = json->TryGetStringField(TEXT("PerformanceRealization"), strContainer) ? FName(strContainer) : "";

	const auto conditionJson = json->GetObjectField(TEXT("ConditionRule"));
	t->mCondition = UCiFRule::loadFromJson(conditionJson, worldContextObject);

	const auto changeJson = json->GetObjectField(TEXT("ChangeRule"));
	t->mChange = UCiFRule::loadFromJson(changeJson, worldContextObject);

	// TODO - not sure yet why this is needed - there is an explanation for this somewhere in comments
	// 	trigger.condition.sortPredicates();
	// 	trigger.change.sortPredicates();

	return t;
}
