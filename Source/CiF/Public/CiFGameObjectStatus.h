// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CiFGameObjectStatus.generated.h"

#define INVALID_ACTOR_ID MAX_uint16

struct FStatusArrayWrapper;
class UCiFGameObject;

UENUM(BlueprintType)
enum class EStatus : uint8
{
	// TODO-	the category statuses can be used when we want to remove or
	//			maybe add all the statuses of the same category to a character.
	//			to add support for this the add/remove statuses in gameObject
	//			should be modified to accomodate it

	INVALID,
	
	// positive category statuses
	CAT_FEELING_GOOD,
	CAT_FEELING_GOOD_ABOUT_SOMEONE,
	LAST_POSITIVE_CATEGORY_STATUS,

	// negative category statuses
	CAT_FEELING_BAD,
	CAT_FEELING_BAD_ABOUT_SOMEONE,
	CAT_REPUTATION_BAD,
	LAST_NEGATIVE_CATEGORY_STATUS,

	// item category statuses
	CAT_USABLE_ITEMS_STATUS,
	CAT_HAS_DURATION,
	CAT_URGE_STATUSES,

	LAST_CATEGORY_COUNT,

	// not directed statuses
	FIRST_NOT_DIRECTED_STATUS,
	EMBARRASSED				UMETA(DisplayName="Embarrassed"), 
	HAPPY					UMETA(DisplayName="Happy"),
	CHEERFUL				UMETA(DisplayName="Cheerful"),
	TIPSY					UMETA(DisplayName="Tipsy"),
	DRUNK					UMETA(DisplayName="Drunk"),
	HEARTBROKEN				UMETA(DisplayName="Heartbroken"),
	OFFENDED				UMETA(DisplayName="Offended"),
	VIRGIN					UMETA(DisplayName="Virgin"),
	CONFUSED				UMETA(DisplayName="Confused"),
	SAD						UMETA(DisplayName="Sad"),
	GUILTY					UMETA(DisplayName="Guilty"),
	DISTRACTED				UMETA(DisplayName="Distracted"),
	EXCITED					UMETA(DisplayName="Excited"),
	VULNERABLE				UMETA(DisplayName="Vulnerable"),
	SHAKEN					UMETA(DisplayName="Shaken"),
	ANXIOUS					UMETA(DisplayName="Anxious"),
	BUSY					UMETA(DisplayName="Busy"),
	ENMITY					UMETA(DisplayName="Enmity"),
	LONELY					UMETA(DisplayName="Lonely"),

	// item statuses
	DRINKABLE				UMETA(DisplayName="DRINKABLE"),
	EATABLE					UMETA(DisplayName="EATABLE"),
	HIDEABLE,
	EXAMINABLE,
	USABLE,
	LOCKED					UMETA(DisplayName="LOCKED"),
	UNLOCKED,
	OPEN,
	HIDDEN,

	// knowledge statuses
	KNOWLEDGE_TRUE			UMETA(DisplayName="TRUE"),
	KNOWLEDGE_FALSE			UMETA(DisplayName="FALSE"),
	KNOWLEDGE_ACTIVE		UMETA(DisplayName="ACTIVE"),

	FIRST_URGE_STATUS,
	QUEST_TYPE_URGE,
	CHARACTER_TYPE_URGE,
	ITEM_TYPE_URGE,
	KNOWLEDGE_TYPE_URGE,
	
	// directed statuses
	FIRST_DIRECTED_STATUS,
	GRATEFUL_TOWARD			UMETA(DisplayName="Greatful toward"),
	RESENTFUL_TOWARD		UMETA(DisplayName="Resentful toward"),
	ANGRY_AT				UMETA(DisplayName="Angry at"),
	ENVIES					UMETA(DisplayName="Envies"),
	AFRAID_OF				UMETA(DisplayName="Afraid of"),
	LOVES					UMETA(DisplayName="Loves"),
	ESTRANGED_FROM			UMETA(DisplayName="Estranged from"),
	HATES					UMETA(DisplayName="Hates"),
	MANIPULATING			UMETA(DisplayName="Manipulating"),
	PITIES					UMETA(DisplayName="Pities"),
	ELOPED_WITH				UMETA(DisplayName="Eloped with"),
	IS_RELATED_TO			UMETA(DisplayName="is related to"),

	// people statuses concerning items
	WANTS					UMETA(DisplayName="wants"),
	LIKES					UMETA(DisplayName="likes"),
	DISLIKES				UMETA(DisplayName="dislikes"),
	IS_HOLDING				UMETA(DisplayName="is holding"),

	// directed statuses concerning knowledge
	KNOWS					UMETA(DisplayName="knows"),
	KNOWN_BY				UMETA(DisplayName="known by"),

	// directed concerning items
	HELD_BY					UMETA(DisplayName="held by"),
	WORN_BY					UMETA(DisplayName="worn by"),

	// relationships
	IS_DATING				UMETA(DisplayName="is dating"),
	IS_FRIENDS_WITH			UMETA(DisplayName="is friends with"),
	IS_ENEMIES_WITH			UMETA(DisplayName="is enemies with"),

};

USTRUCT(BlueprintType)
struct FStatusTypesArrayWrapper
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	TArray<EStatus> mStatusTypes;
};

/**
 * Represents the status of cif game objects.
 * For example, if we have an object representing a bottle, the status of this
 * object can be full or empty etc.
 * If the object represents character, the status can be angry and happy etc.
 */
UCLASS()
class CIF_API UCiFGameObjectStatus : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Sets duration to the status only if @mHasDuration set to true
	 */
	UFUNCTION(BlueprintCallable)
	void setDuration(const int32 newDuration);

	/**
	 * Remove @timeElapsed from the remaining time
	 * @param timeElapsed The amount of time to update by
	 * @returns The remaining duration
	 */
	UFUNCTION(BlueprintCallable)
	int32 updateRemainingDuration(const int32 timeElapsed);
	
	UFUNCTION(BlueprintCallable)
	void init(const EStatus type, const int32 initialDuration=0, const FName towards = "");

	static TMap<EStatus, FStatusTypesArrayWrapper> initializeStatusCategoriesMap();
public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EStatus mType; // type of the status
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool mBinary; // indicates if the status involves 2 actors

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName mDirectedTowards = ""; // the CIF game object name that the status directed towards (!= "" iff mBinary==true)

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool mHasDuration; // if this status has duration - sad character can't be happy forever

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 mRemainingDuration; // remaining duration of the status

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 mInitialDuration; // what is the duration this status starts with

	static TMap<EStatus, FStatusTypesArrayWrapper> mStatusCategories;
	inline static int32 DEFAULT_INITIAL_DURATION = 5;

};
