// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CiFRuleRecord.generated.h"

class UCiFInfluenceRule;

UENUM()
enum class ERuleRecordType
{
	MICROTHEORY,
	SOCIAL_EXCHANGE
};

/**
 * A record of influence rules that fire when forming intent. For use for displaying rules that were true;
 */
UCLASS()
class CIF_API UCiFRuleRecord : public UObject
{
	GENERATED_BODY()

public:
	void init(const FName name,
	          const FName initiatorName,
	          const FName responderName,
	          const FName otherName,
	          const ERuleRecordType type,
	          UCiFInfluenceRule* ir);

	/**
	 * @param outStr Fills the string representing the rule record in natural language
	 */
	void toNLG(FString& outStr);

public:
	UCiFInfluenceRule* mInfluenceRule;
	ERuleRecordType mType;
	FName mName;
	FName mInitiator;
	FName mResponder;
	FName mOther;
};
