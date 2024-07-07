// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFCulturalKnowledgeBase.h"
#include "CiFCKBEntry.h"

void UCiFCulturalKnowledgeBase::findItems(const FName character,
                                          TArray<FName>& outputItems,
                                          const ESubjectiveLabel connectionType,
                                          const ETruthLabel label) const
{
	TArray<UCiFCKBEntry*> charAndConnectionMatches;
	TArray<UCiFCKBEntry*> labelMatches;
	UCiFCKBEntry* entry;

	for (int32 i = 0; i < mSubjectiveEntries.Num(); ++i) {
		entry = mSubjectiveEntries[i];
		if (connectionType != ESubjectiveLabel::INVALID) {
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
	if (label != ETruthLabel::INVALID) {
		for (int32 i = 0; i < mGeneralTruthEntries.Num(); ++i) {
			entry = mGeneralTruthEntries[i];
			const auto truthLabelEnum = StaticEnum<ETruthLabel>();
			const auto tailAsEnumVal = static_cast<ETruthLabel>(truthLabelEnum->GetValueByName(entry->mTail));
			if (tailAsEnumVal == label) {
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

UCiFCulturalKnowledgeBase* UCiFCulturalKnowledgeBase::loadFromJson(const TSharedPtr<FJsonObject> json, const UObject* worldContextObject)
{
	auto ckb = NewObject<UCiFCulturalKnowledgeBase>(const_cast<UObject*>(worldContextObject));

	const auto ckbJson = json->GetArrayField("CKB");
	for (const auto ckbEntryJson : ckbJson) {
		auto ckbEntry = NewObject<UCiFCKBEntry>(const_cast<UObject*>(worldContextObject));
		const auto entryTypeEnum = StaticEnum<ECKBLabelType>();
		const auto entryTypeName = FName(ckbEntryJson->AsObject()->GetStringField("_type"));
		const auto entryType = entryTypeName == "" ? ECKBLabelType::INVALID :
			static_cast<ECKBLabelType>(entryTypeEnum->GetValueByName(entryTypeName));

		const auto head = FName(ckbEntryJson->AsObject()->GetStringField("_head"));
		const auto tail = FName(ckbEntryJson->AsObject()->GetStringField("_tail"));
		const auto connectionTypeEnum = StaticEnum<ESubjectiveLabel>();
		const auto connectionTypeName = FName(ckbEntryJson->AsObject()->GetStringField("_connection"));
		const auto connectionType = connectionTypeName == ""
			                            ? ESubjectiveLabel::INVALID
			                            : static_cast<ESubjectiveLabel>(connectionTypeEnum->GetValueByName(connectionTypeName));

		if (entryType == ECKBLabelType::SUBJECTIVE) {
			ckbEntry->initSubjectiveEntry(head, connectionType, tail);
			ckb->mSubjectiveEntries.Add(ckbEntry);
		}
		if (entryType == ECKBLabelType::GENERAL_TRUTH) {
			const auto truthEnum = StaticEnum<ETruthLabel>();
			const auto truth = tail == "" ? ETruthLabel::INVALID : static_cast<ETruthLabel>(truthEnum->GetValueByName(tail));
			ckbEntry->initTruthEntry(head, connectionType, truth);
			ckb->mGeneralTruthEntries.Add(ckbEntry);
		}
	}

	return ckb;
}
