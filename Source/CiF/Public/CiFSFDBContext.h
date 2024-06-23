// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CiFSFDBContext.generated.h"

class UCiFPredicate;

UENUM()
enum class ESFDBContextType : uint8
{
	SOCIAL_GAME,
	TRIGGER,
	STATUS,
};

/**
 * 
 */
UCLASS()
class CIF_API UCiFSFDBContext : public UObject
{
	GENERATED_BODY()

public:

	/* return the type of the context */
	ESFDBContextType getType() const;

	bool isPredicateInChange(const UCiFPredicate* pred,
							 const UCiFGameObject* c1,
							 const UCiFGameObject* c2,
							 const UCiFGameObject* c3);
	
public:

	UPROPERTY()
	int32 mTime; // the time in game in which the event related to the SFDB entry occured 
};
