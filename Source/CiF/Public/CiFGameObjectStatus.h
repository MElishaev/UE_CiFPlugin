// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CiFGameObjectStatus.generated.h"

#define INVALID_ACTOR_ID MAX_uint16

class UCiFGameObject;

UENUM(BlueprintType)
enum class EStatus : uint8
{
	// positive category statuses
	CAT_FEELING_GOOD = 0,
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
	SICK					UMETA(DisplayName="Sick"),
	HAPPY					UMETA(DisplayName="Happy"),

	// item statuses
	DRINKABLE				UMETA(DisplayName="DRINKABLE"),
	EATABLE					UMETA(DisplayName="EATABLE"),
	LOCKED					UMETA(DisplayName="LOCKED"),

	// knowledge statuses
	KNOWLEDGE_TRUE			UMETA(DisplayName="TRUE"),
	KNOWLEDGE_FALSE			UMETA(DisplayName="FALSE"),
	KNOWLEDGE_ACTIVE		UMETA(DisplayName="ACTIVE"),

	// directed statuses
	GRATEFUL_TOWARD			UMETA(DisplayName="GRATEFUL_TOWARD"),
	ANGRY_AT				UMETA(DisplayName="ANGRY_AT"),
	ENVIES					UMETA(DisplayName="ENVIES"),
	AFRAID_OF				UMETA(DisplayName="AFRAID_OF"),

	// people statuses concerning items
	WANTS					UMETA(DisplayName="WANTS"),
	LIKES					UMETA(DisplayName="LIKES"),
	DISLIKES				UMETA(DisplayName="DISLIKES"),
	IS_HOLDING				UMETA(DisplayName="IS_HOLDING"),

	// directed statuses concerning knowledge
	KNOWS					UMETA(DisplayName="KNOWS"),
	KNOWN_BY				UMETA(DisplayName="KNOWN_BY"),

	// directed concerning items
	HELD_BY					UMETA(DisplayName="HELD_BY"),
	WORN_BY					UMETA(DisplayName="WORN_BY"),

	// relationships
	IS_DATING				UMETA(DisplayName="IS_DATING"),
	IS_FRIENDS_WITH			UMETA(DisplayName="IS_FRIENDS_WITH"),
	IS_ENEMIES_WITH			UMETA(DisplayName="IS_ENEMIES_WITH"),

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
	
	
public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EStatus mType; // type of the status
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool mBinary; // indicates if the status involves 2 actors

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UCiFGameObject* mDirectedTowards; // the CIF game object that the status directed towards (!= null iff mBinary==true)

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool mHasDuration; // if this status has duration - sad character can't be happy forever

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 mRemainingDuration; // remaining duration of the status

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 mInitialDuration; // what is the duration this status starts with
};
