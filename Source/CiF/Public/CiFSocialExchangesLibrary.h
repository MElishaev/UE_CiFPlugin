// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CiFSocialExchangesLibrary.generated.h"

class UCiFSocialExchange;
/**
 * 
 */
UCLASS()
class CIF_API UCiFSocialExchangesLibrary : public UObject
{
	GENERATED_BODY()

public:

	void addSocialExchange(UCiFSocialExchange* se);

	void removeSocialExchange(UCiFSocialExchange* se);

	/**
	 * @return The social exchange or nullptr if not exists 
	 */
	UCiFSocialExchange* getSocialExchangeByName(const FName name);

	void loadSocialGamesLibFromJson(const FString& jsonPath, const UObject* worldContextObject);
public:
	UPROPERTY()
	TMap<FName, UCiFSocialExchange*> mSocialExchanges;
	
};
