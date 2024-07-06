// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CiFSFDBContext.generated.h"

class UCiFGameObject;
class UCiFPredicate;

UENUM()
enum class ESFDBContextType : uint8
{
	INVALID,
	SOCIAL_GAME,
	TRIGGER,
	STATUS,
	BACKSTORY
};

/**
 * 
 */
UCLASS(Abstract)
class CIF_API UCiFSFDBContext : public UObject
{
	GENERATED_BODY()

public:
	/* return the type of the context */
	virtual ESFDBContextType getType() const;

	virtual bool isPredicateInChange(const UCiFPredicate* pred,
	                                 const UCiFGameObject* x,
	                                 const UCiFGameObject* y,
	                                 const UCiFGameObject* z);

	void loadFromJson(const TSharedPtr<FJsonObject> json, const UObject* worldContextObject);

	auto operator<(const UCiFSFDBContext& o) const;
public:
	UPROPERTY()
	int32 mTime; // the time in game in which the event related to the SFDB entry occured 
};
