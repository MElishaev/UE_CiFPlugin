// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CiFQuest.generated.h"

class UCiFCast;
class UCiFItem;
class UCiFCharacter;
class UCiFQuestState;
class UCiFRule;

UENUM(BlueprintType)
enum class ECiFQuestType : uint8
{
	ITEM_QUEST,
	ITEM_GAIN_QUEST,
	ITEM_GIVE_QUEST,

	KNOWLEDGE_QUEST,
	KNOWLEDGE_GAIN_QUEST,
	
	RELATIONSHIP_QUEST,
	RELATIONSHIP_START_QUEST,
	RELATIONSHIP_END_QUEST,
	
	STATUS_QUEST,
	STATUS_GAIN_QUEST,
	STATUS_LOSE_QUEST,
	
	STAT_QUEST,
	STAT_UP_QUEST,
	STAT_DOWN_QUEST,
	
	INVALID,
};

/**
 * 
 */
UCLASS()
class CIF_API UCiFQuest : public UObject
{
	GENERATED_BODY()

public:
	FName mName; // the quest name
	ECiFQuestType mType;
	bool mbItemRequired = false; // true if an item is required for the quest
	FName mQuestGiver; 
	FName mQuestCompleter; // who you complete the quest with

	UPROPERTY()
	TArray<UCiFRule*> mIntents; // store the intents of the quest

	UPROPERTY()
	TArray<UCiFRule*> mPreconditions;

	UPROPERTY()
	TArray<UCiFQuestState*> mStartingStates;

	UPROPERTY()
	TArray<UCiFQuestState*> mCompletionStates;
		
	// tags that are used for picking quests and plot points todo - see if should be removed
	// public var charsMentioned:Vector.<String> = new Vector.<String>();
	// public var locsMentioned:Vector.<String> = new Vector.<String>();
		
	// bool used = false; // Whether the quest has been used already or not
	bool mbCompleted = false;
	// int chosenStart = -1; // the ID matching the starting instantiation used
	
	// int volition = 0;
	// int weight = 0;
		
public:
	
	// Check the success and failure conditions to see if the quest is complete
	// todo why this needs to receive these input params? this class holds the quest giver and completer - why not use them instead?
	bool isCompleted(const FName initiator, const FName responder);

	// todo change input to const
	bool checkAllPreconditions(UCiFCharacter* initiator, UCiFCast* cast, UCiFItem* item=nullptr);

	// todo change input to const
	bool checkPreconditions(UCiFCharacter* initiator, UCiFCharacter* responder, UCiFItem* item);
	
	// void getMostSpecificDialogue(TArray<UCiFQuestState*>& statesMatched, const bool checkForEnding, selectedScene:GrailScene=null, FString& outStr)
	// {
	// 	var qState:QuestState;
	// 		
	// 	// find the most specific state we matched. For now we're just choosing the one with the most number of predicates
	// 	UCiFQuestState* chosenState = statesMatched[0];
	// 	if (statesMatched.Num() > 1) {
	// 		for (const auto* state : statesMatched) {
	// 			if (state->mState->mPredicates.Num() >= chosenState->mState->mPredicates.Num())
	// 				chosenState = state;
	// 		}
	// 	}
	// 		
	// 	// Find the correct piece of dialog - find the scene that matches our starting state id
	// 	if (!selectedScene)
	// 		selectedScene = new GrailScene;
	//
	// 	// if we're looking for ending states, need to find the set that matched startingState
	// 	if (checkForEnding)
	// 	{
	// 		for each (var scene:GrailScene in chosenState.scenes)
	// 		{
	// 			if (scene.pairedStateID == this.chosenStart)
	// 			{
	// 				selectedScene = scene;
	// 				break;
	// 			}
	// 		}
	// 	}
	// 	// if it's a starting state, there's only one scene associated with it
	// 	else
	// 	{
	// 		selectedScene = chosenState.scenes[0];
	// 		chosenStart = selectedScene.pairedStateID;
	// 	}
	// 	
	// 	for each (var lod:LineOfDialogue in selectedScene.dialogue.lines)
	// 	{
	// 		outStr += lod.initiatorLine + "\n";
	// 		outStr += lod.responderLine + "\n";
	// 	}
	// }

	// void getStart(initiator:Character, responder:GameObject, FString& outStr)
	// {
	// 	// Look through the starting states, find the most specific one and fire off the dialogue
	// 	var statesMatched:Vector.<QuestState> = new Vector.<QuestState>();
	// 	var resultStr:String = new String();
	// 	var qState:QuestState;
	// 	for each (qState in this.startingStates)
	// 	{
	// 		if (qState.state.evaluate(initiator, responder))
	// 			statesMatched.push(qState);
	// 	}
	// 	
	// 	if (statesMatched.length > 0)
	// 	{
	// 		resultStr = getMostSpecificDialogue(statesMatched, false);
	// 	}
	// 	else
	// 	{
	// 		return ("We didn't find a starting state that matched, Oops! Writers: Need to write a default quest start state");
	// 	}
	// 	
	// 	return resultStr;
	// }
// 		
// 		public function getEnding(initiatorName:String, responderName:String):String
// 		{
// 			var resultStr:String = new String();
// 			var cif:CiFSingleton = CiFSingleton.getInstance();
// 			
// 			var init:Character = cif.cast.getCharByName(initiatorName);
// 			var resp:GameObject = cif.getGameObjByName(responderName);
// 			
// 			var qState:QuestState;
// 			var selectedScene:GrailScene = new GrailScene;
// 			
// 			// Find which states we matched
// 			var statesMatched:Vector.<QuestState> = new Vector.<QuestState>();
// 			
// 			// make a list of states that we've matched from the list of completion states
// 			for each (qState in this.completionStates)
// 			{
// 				if (qState.state.evaluate(init, resp))
// 					statesMatched.push(qState);
// 			}
// 			
// 			if (statesMatched.length > 0)
// 			{
// 				var ggm:GrailGM = GrailGM.getInstance();
// 				
// 				this.used = true;
// 				if (ggm.questLib.questMixing > 0.3)
// 				{
// 					ggm.questLib.questMixing -= 2 / ggm.questLib.totalQuests;
// 					ggm.questLib.questConcentration += 2 / ggm.questLib.totalQuests;					
// 				}
//
// 				resultStr = getMostSpecificDialogue(statesMatched, true, selectedScene);
// 				
// 				// Fire off the consequences
// 				var initiator:GameObject;
// 				var responder:GameObject;
// 				
// 				for each (var pred:Predicate in selectedScene.socialChange.predicates)
// 				{
// 					initiator = cif.getGameObjByName(pred.first);
// 					responder = cif.getGameObjByName(pred.second);
// 					pred.valuation(initiator, responder, null, null);
// 					trace ("outcome: " + pred.toString() + "\n");
// 				}				
// 			}
// 			else
// 			{
// 				resultStr = "Quest completion state not found. Abort, Abort!";
// 			}
// 			return resultStr;
// 		}
//
//
//
// 		
// 		/**
// 		 * Evaluates the intents of the quest with respect to 
// 		 * character/role mapping given in the arguments.
// 		 * 
// 		 * @param	initiator	The quest giver
// 		 * @return True if all intent rules evaluate to true. False if 
// 		 * they do not.
// 		 */
// 		public function checkIntents(initiator:Character):Boolean {
// 			for each (var intentRule:Rule in this.intents) {
// 				if (!intentRule.evaluate(initiator, null))
// 					return false;
// 			}
// 			return true;
// 		}
//
// 		/**
// 		 * Finds an instantiation given an id. 
// 		 * @param	id	id of instantiation to find.
// 		 * @return	Instantiation with the parameterized id, null if not
// 		 * found.
// 		 */
// /*		public function getInstantiationById(id:int):Instantiation {
// 			for each(var inst:Instantiation in this.instantiations) {
// 				if (id == inst.id)
// 					return inst;
// 			}
// 			Debug.debug(this, "getInstiationById() id not found. id=" + id);
// 			return null;
// 		}
// */	
// 	}

};
