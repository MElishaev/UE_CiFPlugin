// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFInfluenceRuleSet.h"
#include "CiFKnowledge.h"
#include "CiFItem.h"
#include "CiFCast.h"
#include "CiFManager.h"
#include "CiFInfluenceRule.h"
#include "CiFProspectiveMemory.h"
#include "CiFRuleRecord.h"
#include "CiFSocialExchange.h"
#include "CiFSubsystem.h"
#include "Kismet/GameplayStatics.h"

float UCiFInfluenceRuleSet::scoreRules(UCiFCharacter* initiator,
                                       UCiFGameObject* responder,
                                       UCiFGameObject* other,
                                       UCiFSocialExchange* se,
                                       TArray<UCiFGameObject*> activeOtherCast,
                                       FName microtheoryName,
                                       bool isResponder)
{
	float score = 0;
	
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
					if (isResponder && (responder->mGameObjectType == ECiFGameObjectType::CHARACTER)) {
						static_cast<UCiFCharacter*>(responder)->mProspectiveMemory->mResponseSeRuleRecords.Add(rr);
					}
					else if (initiator->mGameObjectType == ECiFGameObjectType::CHARACTER) {
						initiator->mProspectiveMemory->mResponseSeRuleRecords.Add(rr);
					}

					score += ir->mWeight;
					mLastScore.Add(score);
					mLastTruthValues.Add(true);
					mTruthCount++;
				}
				else {
					mLastTruthValues.Add(false);
					mLastScore.Add(0);					
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

					if (isResponder && (responder->mGameObjectType == ECiFGameObjectType::CHARACTER)) {
						static_cast<UCiFCharacter*>(responder)->mProspectiveMemory->mResponseSeRuleRecords.Add(rr);
					}
					else if (initiator->mGameObjectType == ECiFGameObjectType::CHARACTER) {
						initiator->mProspectiveMemory->mResponseSeRuleRecords.Add(rr);
					}

					score += ir->mWeight;
					mLastScore.Add(score);
					mLastTruthValues.Add(true);
					mTruthCount++;
				}
				else {
					mLastTruthValues.Add(false);
					mLastScore.Add(0);					
				}
			}
		}
	}
	
	return score;
}

float UCiFInfluenceRuleSet::scoreRulesWithVariableOther(UCiFCharacter* initiator,
                                                        UCiFGameObject* responder,
                                                        UCiFGameObject* other,
                                                        UCiFSocialExchange* se,
                                                        TArray<UCiFGameObject*> activeOtherCast,
                                                        FName microtheoryName,
                                                        bool isResponder)
{
	float score = 0;

	TArray<UCiFGameObject*> possibleOthers;
	if (!activeOtherCast.IsEmpty()) {
		possibleOthers = activeOtherCast;
	}
	else {
		if (!GetWorld()) {
			UE_LOG(LogTemp, Error, TEXT("Couldn't obtain world"));
			return 0;
		}
		const UCiFManager* cifManager = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UCiFSubsystem>()->getInstance();
		for (const auto c : cifManager->mCast->mCharacters) possibleOthers.Add(c);
		for (const auto c : cifManager->mItemArray) possibleOthers.Add(c);
		for (const auto c : cifManager->mKnowledgeArray) possibleOthers.Add(c);
	}
	
	for (auto ir : mInfluenceRules) {
		if (ir->mWeight != 0) {
			if (ir->isRoleRequired("other")) {
				for (auto o : possibleOthers) {
					if ((o->mObjectName != initiator->mObjectName) && (o->mObjectName != responder->mObjectName)) {
						if (ir->evaluate(initiator, responder, other, se)) {
							auto rr = NewObject<UCiFRuleRecord>();
							auto name = (microtheoryName != "") ? microtheoryName : se->mName;
							auto type = (microtheoryName != "") ? ERuleRecordType::MICROTHEORY : ERuleRecordType::SOCIAL_EXCHANGE;
							rr->init(name, initiator->mObjectName, responder->mObjectName, o->mObjectName, type, ir);
							if (isResponder && (responder->mGameObjectType == ECiFGameObjectType::CHARACTER)) {
								static_cast<UCiFCharacter*>(responder)->mProspectiveMemory->mResponseSeRuleRecords.Add(rr);
							}
							else if (initiator->mGameObjectType == ECiFGameObjectType::CHARACTER) {
								initiator->mProspectiveMemory->mResponseSeRuleRecords.Add(rr);
							}

							score += ir->mWeight;
							mLastScore.Add(score);
							mLastTruthValues.Add(true);
							mTruthCount++;
						}
						else {
							mLastTruthValues.Add(false);
							mLastScore.Add(0);					
						}
					}
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

					if (isResponder && (responder->mGameObjectType == ECiFGameObjectType::CHARACTER)) {
						static_cast<UCiFCharacter*>(responder)->mProspectiveMemory->mResponseSeRuleRecords.Add(rr);
					}
					else if (initiator->mGameObjectType == ECiFGameObjectType::CHARACTER) {
						initiator->mProspectiveMemory->mResponseSeRuleRecords.Add(rr);
					}

					score += ir->mWeight;
					mLastScore.Add(score);
					mLastTruthValues.Add(true);
					mTruthCount++;
				}
				else {
					mLastTruthValues.Add(false);
					mLastScore.Add(0);					
				}
			}
		}
	}
	
	return score;
}
