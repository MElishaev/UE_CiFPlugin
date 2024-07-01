// Fill out your copyright notice in the Description page of Project Settings.


#include "ReadWriteFiles.h"
#include "Json.h"

bool UReadWriteFiles::readStringFromFile(const FString& filePath, FString& outString)
{
	// check if file exists
	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*filePath)) {
		UE_LOG(LogTemp, Error, TEXT("Couldn't find the file at path %s"), *filePath);
		return false;
	}

	// try to read the file
	if (!FFileHelper::LoadFileToString(outString, *filePath)) {
		UE_LOG(LogTemp, Error, TEXT("Couldn't read the file at %s. Is it a text file (ANSI/Unicode formatted)?"), *filePath);
		return false;
	}

	return true;
}

bool UReadWriteFiles::writeStringToFile(const FString& filePath, FString& data)
{
	if (!FFileHelper::SaveStringToFile(data, *filePath)) {
		UE_LOG(LogTemp,
		       Error,
		       TEXT("Couldn't write the file to %s. Check path (legal characters in name) validity, file is available to use and support write access"),
		       *filePath);
		return false;
	}

	return true;
}

bool UReadWriteFiles::readJson(const FString& filePath, TSharedPtr<FJsonObject>& outputJson)
{
	FString jsonString;
	if (!readStringFromFile(filePath, jsonString)) {
		return false;
	}

	if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(jsonString), outputJson)) {
		UE_LOG(LogTemp, Error, TEXT("Couldn't read the json at %s. Is the file in correct format without errors?"), *filePath);
		return false;
	}

	return true;
}

bool UReadWriteFiles::writeJson(const FString& filePath, TSharedPtr<FJsonObject> jsonObject)
{
	FString jsonString;
	if (!FJsonSerializer::Serialize(jsonObject.ToSharedRef(), TJsonWriterFactory<>::Create(&jsonString, 0))) {
		UE_LOG(LogTemp, Error, TEXT("Couldn't write to json at %s. JSON object given may be not serializable"), *filePath);
		return false;
	}

	if (!writeStringToFile(filePath, jsonString)) {
		return false;
	}
	
	return true;
}
