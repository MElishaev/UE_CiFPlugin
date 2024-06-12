// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CiFGameObject.generated.h"

enum class EStatus : uint8;
class UCiFGameObjectStatus;

UENUM(BlueprintType)
enum class ETrait : uint8
{
	// positive traits
	KIND			UMETA(DisplayName="KIND"),
	HUMBLE			UMETA(DisplayName="HUMBLE"),
	LAST_POS_TRAIT,

	// negative traits
	SHY				UMETA(DisplayName="SHY"),
	JEALOUS			UMETA(DisplayName="JEALOUS"),
	LAST_NEG_TRAIT,

	// neutral traits
	MALE			UMETA(DisplayName="MALE"),
	FEMALE			UMETA(DisplayName="FEMALE"),
	LAST_NEUTRAL_TRAIT,

	// item traits
	ITEM			UMETA(DisplayName="ITEM"), // indicates that it is an item
	HOLDABLE		UMETA(DisplayName="HOLDABLE"),
	USABLE			UMETA(DisplayName="USABLE"),

	// knowledge traits
	KNOWLEDGE		UMETA(DisplayName="KNOWLEDGE"), // indicates that this GameObject represents knowledge
	PERSONAL		UMETA(DisplayName="PERSONAL"),
	SECRET			UMETA(DisplayName="SECRET"),
	EMOTIONAL		UMETA(DisplayName="EMOTIONAL"),

	// plot point
	PLOT_POINT		UMETA(DisplayName="PLOT_POINT"), // indicates that this GameObject represents a plot point TODO-- is this necessary?
};

UENUM(BlueprintType)
enum class ECiFGameObjectType : uint8
{
	CHARACTER		UMETA(DisplayName="CHARACTER"),
	ITEM			UMETA(DisplayName="ITEM"),
	KNOWLEDGE		UMETA(DisplayName="KNOWLEDGE")
};

/**
 * This is an abstract base class for a CiF game object.
 * cif game object can be characters, in world objects, knowledge (as in Mismanor)
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Abstract )
class CIF_API UCiFGameObject : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCiFGameObject();

	// TODO-- need to add init method, but i have a problem that i can't overload this method in the child classes when using UFUNCTION

	UFUNCTION(BlueprintCallable)
	void addTrait(const ETrait trait);

	UFUNCTION(BlueprintCallable)
	bool hasTrait(const ETrait trait);	

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) // TODO-- this and more members can be exposed on spawn or something
	FString mObjectName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSet<ETrait> mTraits; // Set of traits of this game object

	// TODO-- not sure this works - can you forward declare status?
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<EStatus, UCiFGameObjectStatus*> mStatuses; // Map of statuses that currently the object has

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	ECiFGameObjectType mGameObjectType; // This game object's type 
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	uint8 mNetworkId; // The ID that this character is represented by in a social network.

	
};
