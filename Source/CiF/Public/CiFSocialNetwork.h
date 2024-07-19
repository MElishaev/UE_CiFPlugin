// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CiFSocialNetwork.generated.h"

UENUM(BlueprintType)
enum class ESocialNetworkType : uint8
{
	// social networks (SN) are weights of feelings of characters towards each other, bidirectional
	BUDDY			UMETA(DisplayName="Buddy"),
	ROMANCE			UMETA(DisplayName="Romance"),
	TRUST,
	FAMILY_BOND,
	LAST_SN_TYPE, // borders the social network types - can be used for comparisons to check if enum represent a social network or relationship

	// relationship network (RN) represent publicly recognized social relationships between characters (friends - true or false)
	RELATIONSHIP		UMETA(DisplayName="Relationship"), 
	SIZE
};

/**
 * This enum describes the relationships that represents statuses.
 * This types are binary true or false relationships and they are dual sided, meaning
 * that if A is friend of B than B also friend of A.
 */
UENUM(BlueprintType)
enum class ERelationshipType : uint8
{
	FRIENDS			UMETA(DisplayName="FRIENDS"),
	DATING			UMETA(DisplayName="DATING"),
	ENEMIES			UMETA(DisplayName="ENEMIES"), 
	SIZE
};

/**
 * Social network is a complete bidirectional graph of some type of social connections
 * between all of the actors in the game. Generally it represents a non public connections,
 * meaning given 2 actors A,B, Trust[A][B] represents how A trusts B, but that doesnt
 * mean that B trusts A the same way or even knows how much A trusts him.
 */
UCLASS()
class CIF_API UCiFSocialNetwork : public UObject
{
	GENERATED_BODY()

public:
	
	/**
	 * @param networkType		The network type of this instance
	 * @param numOfCharacters	The number of characters in the game
	 * @param maxVal			The maximum value of a weight in the matrix
	 */
	UFUNCTION(BlueprintCallable)
	void init(const ESocialNetworkType networkType, const uint8 numOfCharacters, const uint8 maxVal);


	/**
	 * Methods to manipulate the weight at a specific element.
	 * This is clamped to the max value if overflows
	 */
	UFUNCTION(BlueprintCallable)
	void setWeight(const uint8 c1, const uint8 c2, const uint8 w);
	UFUNCTION(BlueprintCallable)
	void addWeight(const uint8 c1, const uint8 c2, const int addition);
	UFUNCTION(BlueprintCallable)
	void multiplyWeight(const uint8 c1, const uint8 c2, const float multiplier);

	UFUNCTION(BlueprintCallable)
	uint8 getWeight(const uint8 c1, const uint8 c2);

	/**
	 * @param c The character in question
	 * @return The average weight of all characters toward this character
	 */
	UFUNCTION(BlueprintCallable)
	float getAverageOpinion(const uint8 c);

	/**
	 * @param c The character we want to query for his relationship towards others
	 * @param th The threshold of which we want the relationships to be above
	 * @return Array of character IDs which @c has relationship higher than threshold towards them
	 */
	UFUNCTION(BlueprintCallable)
	TArray<uint8> getRelationshipsAboveThreshold(const uint8 c, const uint8 th);

	UFUNCTION(BlueprintCallable)
	TArray<uint8> getReverseRelationshipsAboveThreshold(const uint8 c, const uint8 th);

	static UCiFSocialNetwork* loadFromJson(const TSharedPtr<FJsonObject> json, const UObject* worldContextObject);
protected:
	void setAllArrayElements(uint8 val);

	
public:

	TArray<TArray<uint8>> mNetwork; // represents 2d array of relationship value where Network[x][y] is the opinion of x towards y

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESocialNetworkType mType;

	uint8 mMaxVal; // the max value a network edge can hold

	
};
