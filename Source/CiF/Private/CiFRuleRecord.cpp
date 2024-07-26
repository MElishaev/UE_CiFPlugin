// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFRuleRecord.h"

#include "CiFInfluenceRule.h"

void UCiFRuleRecord::init(const FName name,
                          const FName initiatorName,
                          const FName responderName,
                          const FName otherName,
                          const ERuleRecordType type,
                          UCiFInfluenceRule* ir)
{
	mName = name;
	mInitiator = initiatorName;
	mResponder = responderName;
	mOther = otherName;
	mType = type;
	mInfluenceRule = ir;
}

void UCiFRuleRecord::toNLG(FString& outStr)
{
	mInfluenceRule->toNLG(outStr, mInitiator, mResponder, mOther);
}
