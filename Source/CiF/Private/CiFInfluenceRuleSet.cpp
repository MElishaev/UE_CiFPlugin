// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFInfluenceRuleSet.h"
#include "CiFSocialExchange.h"

float UCiFInfluenceRuleSet::scoreRules(UCiFCharacter* initiator,
                                       UCiFGameObject* responder,
                                       UCiFGameObject* other,
                                       UCiFSocialExchange* se,
                                       TArray<UCiFGameObject*> activeOtherCast,
                                       FName microtheoryName,
                                       bool isResponder)
{
	// TODO - implement
	return 0;
}

float UCiFInfluenceRuleSet::scoreRulesWithVariableOther(UCiFCharacter* initiator,
                                                        UCiFGameObject* responder,
                                                        UCiFGameObject* other,
                                                        UCiFSocialExchange* se,
                                                        TArray<UCiFGameObject*> activeOtherCast,
                                                        FName microtheoryName,
                                                        bool isResponder)
{
	// TODO - implement
	return 0;
}
