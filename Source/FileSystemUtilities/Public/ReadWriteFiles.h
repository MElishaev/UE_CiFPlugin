// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ReadWriteFiles.generated.h"

class FJsonObject;

/**
 * 
 */
UCLASS()
class FILESYSTEMUTILITIES_API UReadWriteFiles : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Reads some type of text file and stores its contents in @outString
	 * @param filePath The file path to read
	 * @param outString The read string will be stored here
	 * @return True if was successful reading the file or false otherwise
	 */
	UFUNCTION(BlueprintCallable)
	static bool readStringFromFile(const FString& filePath, FString& outString);

	/**
	 * Writes string to file
	 * @param filePath The file path to read
	 * @param data The data to write into the file
	 * @return True if was successful writing the file or false otherwise
	 */
	UFUNCTION(BlueprintCallable)
	static bool writeStringToFile(const FString& filePath, FString& data);

	/**
	 * Reads json file given in path and stores in UE json object
	 * @param filePath file path of the json file
	 * @param outputJson pointer to a populated json object
	 * @return true if read successfully
	 */
	static bool readJson(const FString& filePath, TSharedPtr<FJsonObject>& outputJson);

	/**
	 * Writes json object into a json file in the specified given path if possible
	 * @param filePath file path for writing the output
	 * @param jsonObject pointer to the json object to write
	 * @return true if written successfully
	 */
	static bool writeJson(const FString& filePath, TSharedPtr<FJsonObject> jsonObject);
};
