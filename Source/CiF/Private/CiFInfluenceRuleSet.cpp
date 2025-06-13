// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFInfluenceRuleSet.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "CiFManager.h"
#include "CiFInfluenceRule.h"
#include "CiFProspectiveMemory.h"
#include "CiFRuleRecord.h"
#include "CiFSocialExchange.h"
#include "CiFSubsystem.h"

float UCiFInfluenceRuleSet::scoreRules(UCiFCharacter* initiator,
                                       UCiFGameObject* responder,
                                       UCiFGameObject* other,
                                       UCiFSocialExchange* se,
                                       FName microtheoryName,
                                       bool isResponder)
{
	FScore_t score;

	for (auto ir : mInfluenceRules) {
		if (ir->mWeight != 0) {
			if (ir->isRoleRequired("other")) {
				if (!other) {
					UE_LOG(LogTemp, Error, TEXT("No other was passed in while needed"));
					return 0;
				}

				if (ir->evaluate(initiator, responder, other, se)) {
					auto rr = NewObject<UCiFRuleRecord>();
					auto name = (microtheoryName != "") ? microtheoryName : se->mName;
					auto type = (microtheoryName != "") ? ERuleRecordType::MICROTHEORY : ERuleRecordType::SOCIAL_EXCHANGE;
					rr->init(name, initiator->mObjectName, responder->mObjectName, other->mObjectName, type, ir);

					FRRMapKey key = FRRMapKey(name, initiator->mObjectName, responder->mObjectName, other->mObjectName);
					if (isResponder && (responder->mGameObjectType == ECiFGameObjectType::CHARACTER)) {
						static_cast<UCiFCharacter*>(responder)->mProspectiveMemory->storeRuleRecord(key, rr);
					}
					else if (initiator->mGameObjectType == ECiFGameObjectType::CHARACTER) {
						initiator->mProspectiveMemory->storeRuleRecord(key, rr);
					}

					score += ir->mWeight;
				}
			}
			else {
				// no other required
				if (ir->evaluate(initiator, responder, nullptr, se)) {
					auto rr = NewObject<UCiFRuleRecord>();
					auto name = (microtheoryName != "") ? microtheoryName : se->mName;
					auto type = (microtheoryName != "") ? ERuleRecordType::MICROTHEORY : ERuleRecordType::SOCIAL_EXCHANGE;
					// if there is an other, this means it is the other that was important in the SG precondition or MT def
					auto otherName = other ? other->mObjectName : "";
					rr->init(name, initiator->mObjectName, responder->mObjectName, otherName, type, ir);

					FRRMapKey key = FRRMapKey(name, initiator->mObjectName, responder->mObjectName, otherName);
					if (isResponder && (responder->mGameObjectType == ECiFGameObjectType::CHARACTER)) {
						static_cast<UCiFCharacter*>(responder)->mProspectiveMemory->storeRuleRecord(key, rr);
					}
					else if (initiator->mGameObjectType == ECiFGameObjectType::CHARACTER) {
						initiator->mProspectiveMemory->storeRuleRecord(key, rr);
					}

					score += ir->mWeight;
				}
			}
		}
	}

	return score;
}

FScore_t UCiFInfluenceRuleSet::scoreRulesWithVariableOther(UCiFCharacter* initiator,
                                                           UCiFGameObject* responder,
                                                           UCiFGameObject* other,
                                                           UCiFSocialExchange* se,
                                                           TArray<UCiFGameObject*> activeOtherCast,
                                                           FName microtheoryName,
                                                           bool isResponder)
{
	FScore_t score = 0; // todo - why the score is global and not per other?

	TArray<UCiFGameObject*> possibleOthers;
	if (!activeOtherCast.IsEmpty()) {
		possibleOthers = activeOtherCast;
	}
	else {
		const UCiFManager* cifManager = GetWorld()->GetGameInstance()->GetSubsystem<UCiFSubsystem>()->getInstance();
		cifManager->getAllGameObjects(possibleOthers);
	}
	
	for (auto ir : mInfluenceRules) {
		if (ir->mWeight != 0) {
			if (ir->isRoleRequired("other")) {
				for (auto o : possibleOthers) {
					if ((o->mObjectName != initiator->mObjectName) && (o->mObjectName != responder->mObjectName)) {
						if (ir->evaluate(initiator, responder, o, se)) {
							/* create and store IR that evaluated to true as a rule record in the prospective memory */
							auto rr = NewObject<UCiFRuleRecord>();
							const auto name = (microtheoryName != NAME_None) ? microtheoryName : se->mName;
							const auto type = (microtheoryName != NAME_None)
								                  ? ERuleRecordType::MICROTHEORY
								                  : ERuleRecordType::SOCIAL_EXCHANGE;
							rr->init(name, initiator->mObjectName, responder->mObjectName, o->mObjectName, type, ir);

							const auto key = FRRMapKey(se->mName, initiator->mObjectName, responder->mObjectName, o->mObjectName);
							if (isResponder && (responder->mGameObjectType == ECiFGameObjectType::CHARACTER)) {
								static_cast<UCiFCharacter*>(responder)->mProspectiveMemory->storeRuleRecord(key, rr);
							}
							else if (initiator->mGameObjectType == ECiFGameObjectType::CHARACTER) {
								initiator->mProspectiveMemory->storeRuleRecord(key, rr);
							}

							score += ir->mWeight;
						}
					}
				}
			}
			else {
				// no other required
				if (ir->evaluate(initiator, responder, nullptr, se)) {
					auto rr = NewObject<UCiFRuleRecord>();
					const auto name = (microtheoryName != NAME_None) ? microtheoryName : se->mName;
					const auto type = (microtheoryName != NAME_None) ? ERuleRecordType::MICROTHEORY : ERuleRecordType::SOCIAL_EXCHANGE;
					// if there is an other, this means it is the other that was important in the SG precondition or MT def
					const auto otherName = other ? other->mObjectName : NAME_None;
					rr->init(name, initiator->mObjectName, responder->mObjectName, otherName, type, ir);

					const auto key = FRRMapKey(se->mName, initiator->mObjectName, responder->mObjectName, otherName);
					if (isResponder && (responder->mGameObjectType == ECiFGameObjectType::CHARACTER)) {
						static_cast<UCiFCharacter*>(responder)->mProspectiveMemory->storeRuleRecord(key, rr);
					}
					else if (initiator->mGameObjectType == ECiFGameObjectType::CHARACTER) {
						initiator->mProspectiveMemory->storeRuleRecord(key, rr);
					}
					
					score += ir->mWeight;
				}
			}
		}
	}
		
	return score;
}
