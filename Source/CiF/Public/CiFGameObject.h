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

USTRUCT(BlueprintType)
struct FStatusArrayWrapper
{
	GENERATED_BODY()

	TArray<UCiFGameObjectStatus*> statusArray;	
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
	bool hasTrait(const ETrait trait) const;	

	/**
	 * Determines if the game object has a status of given type or if @towards
	 * param given, checks if the status is towards the specified game object
	 * @param	statusType		The type of the status.
	 * @param	towards			The character the status is directed to. null if not looking for directed status
	 * @return	True if the character has the status, false if he does not.
	 */
	UFUNCTION(BlueprintCallable)
	bool hasStatus(const EStatus statusType, const UCiFGameObject* towards=nullptr) const;

	/**
	 * Give the character a status with a type and a character status target if the status is directed.
	 * @param statusType		The type of the status.
	 * @param duration			Initial duration of the status
	 * @param towards			The character the status is directed to.
	 */
	UFUNCTION(BlueprintCallable)
	void addStatus(const EStatus statusType, const int32 duration=0, UCiFGameObject* towards=nullptr);

	/**
	 * Removes a status from the character according to status type.
	 * Currently removes only 1 status and doesn't support removal of all statuses
	 * of a specific category (maybe add support later TODO)
	 * @param statusType	The type of status to remove.
	 * @param towards		Optional parameter for directed statuses
	 */
	UFUNCTION(BlueprintCallable)
	void removeStatus(const EStatus statusType, UCiFGameObject* towards=nullptr);

	/**
	 * Updates the duration of all statuses held by the character. Removes
	 * statuses that have 0 or less remaining duration.
	 * TODO: add the status removal to the SFDB.
	 * @param	timeElapsed	The amount of time to remove from the statuses.
	 */
	UFUNCTION(BlueprintCallable)
	void updateStatusDurations(const int32 timeElapsed=1); 
	
	/**
	 * @return The status or null if doesn't exists 
	 */
	UFUNCTION(BlueprintCallable)
	UCiFGameObjectStatus* getStatus(const EStatus statusType, const UCiFGameObject* towards=nullptr);
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) // TODO-- this and more members can be exposed on spawn or something
	FName mObjectName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSet<ETrait> mTraits; // Set of traits of this game object

	// TODO-- not sure this works - can you forward declare status?
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<EStatus, FStatusArrayWrapper> mStatuses; // Map of statuses that currently the object has

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	ECiFGameObjectType mGameObjectType; // This game object's type 
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	uint8 mNetworkId; // The ID that this character is represented by in a social network.

	
};
