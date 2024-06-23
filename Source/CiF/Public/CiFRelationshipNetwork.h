// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CiFSocialNetwork.h"
#include "CiFRelationshipNetwork.generated.h"

class UCiFCharacter;
/**
 * 
 */
UCLASS()
class CIF_API UCiFRelationshipNetwork : public UCiFSocialNetwork
{
	GENERATED_BODY()

public:

	/**
	 * @param numOfCharacters	The number of characters in the game
	 * @param maxVal			The maximum value of a weight in the matrix
	 */
	UFUNCTION(BlueprintCallable)
	void initialize(const uint8 numOfCharacters, const uint8 maxVal);
	
	/**
	 * Checks for a relationship between two characters. It always checks if
	 * the character of the second parameter has the relationship denoted by
	 * the first parameter with the character represented by the third
	 * parameter. Reciprocal and directional relationships are treated
	 * identically.
	 *  
	 * @param	relationship The relationship type for the relationship to be checked.
	 * @param	a The Character for which the relationship is checked.
	 * @param	b The Character that is the object of the relationship check.
	 * @return True if the relationship is present from a to b. False if it is
	 * not.
	 */
	UFUNCTION(BlueprintCallable)
	bool getRelationship(const ERelationshipType relationship, const UCiFCharacter* a, const UCiFCharacter* b);


	/** 
	 * Removes a relationship from the character playing role A to the
	 * character playing role B. If the relationship a reciprocal one (i.e
	 * dating, friends, enemies etc.), the network values are set
	 * bidirectionally. If the relationship is a directed one, it is set in the
	 * direction of A to B.
	 * 
	 * @param	relationship The relationship value for the relationship to be removed.
	 * @param	a The Character for which the relationship is modified.
	 * @param	b The Character that is the object of the relationship change.
	 */
	UFUNCTION(BlueprintCallable)
	void removeRelationship(const ERelationshipType relationship, const UCiFCharacter* a, const UCiFCharacter* b);
	
	/**
	 * Sets a relationship from the character playing role A to the
	 * character playing role B. If the relationship a reciprocal one (i.e
	 * dating, friends or enemies), the network values are set
	 * bidirectionally. If the relationship is a directed one, it is set in the
	 * direction of A to B.
	 * 
	 * @param	relationship The Relationship value for the Relationship to be added.
	 * @param	a The Character for which the Relationship is true.
	 * @param	b The Character that is the object of the Relationship.
	 */
	UFUNCTION(BlueprintCallable)
	void setRelationship(const ERelationshipType relationship, const UCiFCharacter* a, const UCiFCharacter* b);
};
