// Fill out your copyright notice in the Description page of Project Settings.


#include "CiFGameObject.h"

// Sets default values for this component's properties
UCiFGameObject::UCiFGameObject()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

void UCiFGameObject::addTrait(const ETrait trait)
{
	mTraits.Add(trait);
}

bool UCiFGameObject::hasTrait(const ETrait trait)
{
	return mTraits.Contains(trait);
}


// Called when the game starts
void UCiFGameObject::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UCiFGameObject::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

