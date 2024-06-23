// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CiFSocialFactsDataBase.generated.h"

class UCiFPredicate;
class UCiFGameObject;
class UCiFSFDBContext;

UENUM(BlueprintType)
enum class ESFDBLabelType : uint8
{
	// Example for categories of social facts - can expand on this
	CAT_POSITIVE,
	CAT_NEGATIVE,
	CAT_FLIRT,
	CAT_LAST, ///< indicates the last category type for arithmetic comparisons

	// Labels
	LABEL_WILDCARD,
	LABEL_COOL,
	LABEL_MEAN,
	LABEL_TABOO,
	LABEL_RUDE,
	LABEL_LAME,
	LABEL_ROMANTIC,
	LABEL_EMBERASSING,
	LABEL_FAILED_ROMANCE,
	LABEL_FUNNY,
	SIZE
};

/**
 * A wrapper struct for an array that will be placed inside a static map
 * to arrange the SFDB labels into categories
 */
USTRUCT(BlueprintType)
struct FLabelCategoryArrayWrapper
{
	GENERATED_BODY()
	TArray<ESFDBLabelType> mCategoryLabels;
};

USTRUCT(BlueprintType)
struct FSFDBLabel
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Test Variables")
	FName from;
	FName to = "";
	ESFDBLabelType type;

	bool operator==(const FSFDBLabel& other) const
	{
		return from == other.from &&
			to == other.to &&
			type == other.type;
	}
};

USTRUCT(BlueprintType)
struct FSFDBEntry
{
	GENERATED_BODY()

	FName exchangeName;
	FName initiator;
	FName responder;
	float initiatorScore;
	float responderScore;
	int32 time;            // at what time this exchange happened
	int32 effectID;        // indicates which specific instantiation of Bully happened (in this case, making fun of SAT scores)
	ESFDBLabelType type; // todo: maybe should be an array - would be more clear later when using this system
};

/**
 * An entry in the knowledge base should look something like this:
 * (SocialGameContext exchangeName = “Bully” initiator = “Edward” responder = “Chloe”
 * initiatorScore = “15” responderScore = “10” time = “5” effectID = “10” other = “”(SFKBLabel type = “mean”))
 */
UCLASS()
class CIF_API UCiFSocialFactsDataBase : public UObject
{
	GENERATED_BODY()

public:
	// Returns the oldest context time w.r.t game start time. Could be negative due to authored backstory which included before game starts
	int32 getLowestContextTime() const;

	// Returns timestamp of the latest SFDB context in game time
	int32 getLatestContextTime() const;

	/**
	 * Returns the most recent time a predicate was true in social exchange associated with a SFDBContext.
	 * 
	 * @param	pred	Predicate to check for.
	 * @param	c1			Primary character.
	 * @param	c2			Secondary character.
	 * @param	c3			Tertiary character.
	 * @return	The time when the predicate was true if found. SocialFactsDB.PREDICATE_NOT_FOUND if not found.
	 */
	int timeOfPredicateInHistory(const UCiFPredicate* pred,
	                             const UCiFGameObject* c1,
	                             const UCiFGameObject* c2,
	                             const UCiFGameObject* c3) const;

	/**
	 * Determines the predicate was true in any social exchange in any SFDBContext within
	 * the time window specified in the predicate p.
	 * 
	 * @param	pred	Predicate to check for.
	 * @param	c1	Primary character.
	 * @param	c2	Secondary character.
	 * @param	c3	Tertiary character.
	 * @return	True if the predicate was found, false if not.
	 */
	bool isPredicateInHistory(const UCiFPredicate* pred,
	                          const UCiFGameObject* c1,
	                          const UCiFGameObject* c2,
	                          const UCiFGameObject* c3) const;

	/**
	 * Returns an array of indices by looking at all the social exchanges
	 * or trigger contexts in the @window, ones that matching the arguments
	 * @type, @c1, @c2
	 * @param outMatchingIndices  Vector of timestamps of matching social facts data base context entries 
	 * @param label		SFDB type to locate
	 * @param c1		First character (from) slot
	 * @param c2		Second character (to) slot
	 * @param window	The window in SFDB time to look back for matches. A window of 0 means the entire history
	 * @param pred 
	 */
	void findLabelFromValues(TArray<int32>& outMatchingIndices,
	                         const ESFDBLabelType label,
	                         const UCiFGameObject* c1 = nullptr,
	                         const UCiFGameObject* c2 = nullptr,
	                         const UCiFGameObject* c3 = nullptr,
	                         int window = 0,
	                         const UCiFPredicate* pred = nullptr) const;

	/**
	 * This function is used to deal with when we are seeing is a label is in a category.
	 * If predicateLabel is not a category, returns false if it doesn't match contextLabel: true otherwise.
	 * If predicateLabel is a category, it will loop through that category to see if contextLabel matches any in the category.
	 * 
	 * @param contextLabel		The context label type of the context
	 * @param predicateLabel	The predicate label type to match to
	 */
	static bool doesMatchLabelOrCategory(const ESFDBLabelType contextLabel, const ESFDBLabelType predicateLabel) noexcept;

	static TMap<ESFDBLabelType, FLabelCategoryArrayWrapper> initializeCategoriesMap();
public:
	TArray<UCiFSFDBContext*> mContexts; // contexts in ascending order - the latest is the last in the array

	static TMap<ESFDBLabelType, FLabelCategoryArrayWrapper> mSFDBLabelCategories;
};
