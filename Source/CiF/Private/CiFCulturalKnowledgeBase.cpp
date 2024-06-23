// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFCulturalKnowledgeBase.h"
#include "CiFCKBEntry.h"

void UCiFCulturalKnowledgeBase::findItems(const FName character, TArray<FName>& outputItems, const FName connectionType, const FName label) const
{
	TArray<UCiFCKBEntry*> charAndConnectionMatches;
	TArray<UCiFCKBEntry*> labelMatches;
	UCiFCKBEntry* entry;
	
	for (int32 i = 0; i < mSubjectiveEntries.Num(); ++i) {
		entry = mSubjectiveEntries[i];
		if (connectionType != "") {
			//if the connection type is not null, match to it.
			if (entry->mHead == character && entry->mConnection == connectionType) {
				charAndConnectionMatches.Push(entry);
			}
		}
		else {
			//if the connection type is null, treat it as a wild card; return all as matches.
			if (entry->mHead == character) {
				charAndConnectionMatches.Push(entry);
			}
		}
	}

	//if the label is not null, match to it.
	//if the label type is null, treat it as a wild card; return all as matches.
	if (label != "") {
		for (int32 i = 0; i < mGeneralTruthEntries.Num(); ++i) {
			entry = mGeneralTruthEntries[i];
			if (entry->mTail == label) {
				labelMatches.Push(entry);
			}
		}
	}
	else {
		labelMatches = mGeneralTruthEntries;
	}

	for (int32 i = 0; i < charAndConnectionMatches.Num(); ++i) {
		for (int32 j = 0; j < labelMatches.Num(); ++j) {
			if (charAndConnectionMatches[i]->mTail == labelMatches[j]->mHead) {
				outputItems.Push(charAndConnectionMatches[i]->mTail);
			}
		}
	}
}
