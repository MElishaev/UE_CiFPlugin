﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CiFCast.generated.h"

class UCiFCharacter;
/**
 * A class that stores the characters participating in the CiF system
 */
UCLASS()
class CIF_API UCiFCast : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * This initialization method should be called after all characters were added
	 */
	void init(UObject* worldContextObject);
	
	/**
	 * @return pointer to the character or nullptr if not exists 
	 */
	UFUNCTION(BlueprintCallable)
	UCiFCharacter* getCharByName(const FName name) const;

	UFUNCTION(BlueprintCallable)
	UCiFCharacter* getCharByNetworkId(const uint8 id) const;
	
	UFUNCTION(BlueprintCallable)
	void addCharacter(UCiFCharacter* c);	

	static UCiFCast* loadFromJson(const TSharedPtr<FJsonObject> json, const UObject* worldContextObject);
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UCiFCharacter*> mCharacters;

	UPROPERTY(BlueprintReadOnly)
	TMap<FName, UCiFCharacter*> mCharactersByName; // for fast lookup - represents the same characters in @mCharacters

	
};
