// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CiFGameObject.h"
#include "CiFItem.generated.h"

UENUM(BlueprintType)
enum class ECiFItemType : uint8
{
	INVALID,
	WINE,
	WHITE_WINE,
	COLONELS_DIARY,
	VIOLETS_DIARY,
	CULT_STATUE,
	GATE
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CIF_API UCiFItem : public UCiFGameObject
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCiFItem();

	UFUNCTION(BlueprintCallable)
	void init(const ECiFItemType itemType, UCiFGameObject* receivedFrom=nullptr);

	static UCiFItem* loadFromJson(const TSharedPtr<FJsonObject> json, const UObject* worldContextObject);
public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UCiFGameObject* mReceivedFrom; // game object that the item was received from. null if didn't receive from anyone

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	ECiFItemType mItemType;

};

