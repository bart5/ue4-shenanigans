// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"
#include "StaticDataFS.h"

UMyGameInstance::UMyGameInstance() {
	UE_LOG(LogTemp, Warning, TEXT("[MYLOG] About to set static data"));

	FSDFSActions::LoadStaticData();
};

//FStaticData UMyGameInstance::GetStaticData() {
//	return FSDStore::StaticData;
//}

