// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFCKBEntry.h"

void UCiFCKBEntry::initSubjectiveEntry(const FName head, const ESubjectiveLabel connectionType, const FName tail)
{
	mHead = head;
	mConnection = connectionType;
	mTail = tail;
	mType = ECKBLabelType::SUBJECTIVE;
}

void UCiFCKBEntry::initTruthEntry(const FName head, const ESubjectiveLabel connectionType, const ETruthLabel tail)
{
	mHead = head;
	mConnection = connectionType;
	const auto truthFName = StaticEnum<ETruthLabel>()->GetNameByValue(static_cast<int64>(tail));
	mTail = truthFName;
	mType = ECKBLabelType::GENERAL_TRUTH;
}
