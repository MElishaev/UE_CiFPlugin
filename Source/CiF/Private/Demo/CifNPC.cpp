// Fill out your copyright notice in the Description page of Project Settings.


#include "Demo/CifNPC.h"


// Sets default values
ACifNPC::ACifNPC()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// mCifCharacterComp = CreateDefaultSubobject<UCiFCharacter>(TEXT("CifCharacterComponent"));
}

// Called when the game starts or when spawned
void ACifNPC::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACifNPC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ACifNPC::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

bool ACifNPC::isSocialGameInHistory(const FName sg) const
{
	return mLastSocialMoves.Contains(sg);
}

