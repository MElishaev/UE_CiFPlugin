// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFRuleRecord.h"

#include "CiFInfluenceRule.h"

void UCiFRuleRecord::init(const FName name,
                          const FName initiatorName,
                          const FName responderName,
                          const FName otherName,
                          const ERuleRecordType type,
                          const UCiFInfluenceRule* ir)
{
	mName = name;
	mInitiator = initiatorName;
	mResponder = responderName;
	mOther = otherName;
	mType = type;
	mInfluenceRule = const_cast<UCiFInfluenceRule*>(ir);
}

void UCiFRuleRecord::toNLG(FString& outStr) const
{
	mInfluenceRule->toNLG(outStr, mInitiator, mResponder, mOther);
}

void UCiFRuleRecord::toDebugNLG(FString& outStr) const
{
	outStr = mType == ERuleRecordType::MICROTHEORY ? "(MT::" : "(SE::";
	outStr += mName.ToString() + ") rule: ";
	toNLG(outStr);
	outStr += " " + FString::FromInt(mInfluenceRule->mWeight);
}
