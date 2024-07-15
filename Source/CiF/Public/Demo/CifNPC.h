// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CiFCharacter.h"
#include "GameFramework/Character.h"
#include "CifNPC.generated.h"

UCLASS()
class CIF_API ACifNPC : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACifNPC();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	bool isSocialGameInHistory(const FName sg) const;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"))
	UCiFCharacter* mCifCharacterComp; // cif character component
	
	TArray<FName> mLastSocialMoves; // the last N moves
	int32 mNumOfLastMoves; // the max number of last moves to store
	
};
