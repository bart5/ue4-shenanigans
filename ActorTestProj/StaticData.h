// 2020 Purple Pill Productions.

#pragma once

#include <any>
#include <vector>
#include <array>
#include <fstream>
#include <sstream>
#include "Misc/DefaultValueHelper.h"
#include "StaticData.generated.h"

enum class EValueTypes {
	int32 = 0 ,
	flt,
	string,
	boolean,
};

enum class ESDTypes {
	type1 = 0,
	type2 = 1,
};

enum class EInstanceAction {
	toString,
	toInstance,
};

struct FType1Data {
	int32 id;
	int32 prop1;
	int32 prop2;
};

struct FType2Data {
	int32 id;
	int32 prop1;
	int32 prop2;
};

template<ESDTypes E>
struct FTypeData {
	int32 id;

	friend int32 GetTypeHash(const FTypeData<E>& myStruct) {
		return myStruct.id;
	};

	friend bool operator==(const FTypeData<E>& LHS, const FTypeData<E>& RHS)
	{
		return LHS.id == RHS.id;
	};
};
template<> struct FTypeData<ESDTypes::type1> : public FType1Data {};
template<> struct FTypeData<ESDTypes::type2> : public FType2Data {};

struct FSDInstanceProp {
	std::string propName;
	EValueTypes propValueType;
	std::vector<std::string> propValues;
	bool isArray = false;
};

FString castStdStringToFstring(std::string value) {
	try
	{
		return FString(value.c_str());
	}
	catch (const std::exception& exc)
	{
		UE_LOG(LogTemp, Warning, TEXT("---------> [!!!!] Failed casting \"%s\" to FString. Exception:\n%s"), value, exc.what());
		return (FString)"0conversionError";
	}
};

int32 castStdStringToInt32(std::string value) {
	try
	{
		FString fstr = castStdStringToFstring(value);
		int32 out;
		FDefaultValueHelper::ParseInt(fstr, out);
		return out;
	}
	catch (const std::exception& exc)
	{
		UE_LOG(LogTemp, Warning, TEXT("---------> [!!!!] Failed casting \"%s\" to int32. Exception:\n%s"), value, exc.what());
		return 0;
	}
};

float castStdStringToFloat(std::string value) {
	try
	{
		std::string::size_type sz;   // alias of size_t
		return std::stof(value, &sz);
	}
	catch (const std::exception& exc)
	{
		UE_LOG(LogTemp, Warning, TEXT("---------> [!!!!] Failed casting \"%s\" to float. Exception:\n%s"), value, exc.what());
		return 0;
	}
};

bool castStdStringToBool(std::string value) {
	if (value == "0") {
		return false;
	}
	else if (value == "1") {
		return true;
	}
	else if (value == "true") {
		return true;
	}
	else if (value == "false") {
		return false;
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("---------> [!!!!] Failed casting \"%s\" to bool."), value);
		return false;
	}
};

// Or template with implicit type argument deduction
template<class T>
void FSDPropertyAction(T& instanceProperty, std::string propName, std::vector<FSDInstanceProp>& properties, EInstanceAction action) {
	if (action == EInstanceAction::toInstance) {
		bool found = false;

		auto GetValueFromString = [](FSDInstanceProp& prop, int32 i = 0) {
			try {
				switch (prop.propValueType)
				{
				case EValueTypes::int32:
					if (!std::is_same<decltype(instanceProperty), int32>::value) throw;
					return castStdStringToInt32(prop.propValues[i]);
				case EValueTypes::flt:
					if (!std::is_same<decltype(instanceProperty), float>::value) throw;
					return castStdStringToFloat(prop.propValues[i]);
				case EValueTypes::string:
					if (!std::is_same<decltype(instanceProperty), FString>::value) throw;
					return castStdStringToFstring(prop.propValues[i]);
				case EValueTypes::boolean:
					if (!std::is_same<decltype(instanceProperty), bool>::value) throw;
					return castStdStringToBool(prop.propValues[i]);
				}
			}
			catch () {
				UE_LOG(LogTemp, Warning, TEXT("---------> [!!!!] Encoded property type doesn't match declared instance property type."));
			}
		};

		for (FSDInstanceProp prop : properties) {
			if (propName == prop.propName) {
				found = true;
				if (prop.isArray) {
					for (int32 i = 0; i < prop.propValues.size(); i++) {
						instanceProperty[i] = GetValueFromString(prop, i);
					}
				}
				else {
					instanceProperty = GetValue(prop, 0);
				}
			}
		}
		if (!found) {
			UE_LOG(LogTemp, Warning, TEXT("---------> [!!!!] Prop of name: \"%s\" not found."), propName);
			// :(
		}
	}
	else {
		FSDInstanceProp prop;
		prop.propName = propName;

		auto encodeValueInString = [](auto& source) {
		std:string out;
			FString fstr;
			if (std::is_same<decltype(source), int32>::value) {
				prop.propValueType = EValueTypes::int32;
				fstr = FString::FromInt(source);
				out = std::string(TCHAR_TO_UTF8(*fstr));
			}
			else if (std::is_same<decltype(source), float>::value) {
				prop.propValueType = EValueTypes::flt;
				fstr = FString::SanitizeFloat(source);
				out = std::string(TCHAR_TO_UTF8(*fstr));
			}
			else if (std::is_same<decltype(source), FString>::value) {
				prop.propValueType = EValueTypes::string;
				out = std::string(TCHAR_TO_UTF8(*source));
			}
			else if (std::is_same<decltype(source), bool>::value) {
				prop.propValueType = EValueTypes::boolean;
				out = source ? "0" : "1";
			}
			else {
				UE_LOG(LogTemp, Warning, TEXT("---------> [!!!!] Instance property type not matching known value types."));
			}
			return out
		};

		if (std::is_same<decltype(instanceProperty), TArray>::value) {
			prop.isArray = true;
			for (auto& value : instanceProperty)
			{
				prop.propValues.push_back(encodeValueInString(value));
			}
		}
		else {
			prop.isArray = false;
			prop.propValues.push_back(encodeValueInString(instanceProperty));
		};

		properties.push_back(prop);
	};
};

typedef std::vector<FSDInstanceProp>& P;
typedef EInstanceAction A;
template<ESDTypes E>
void FSDInstanceAction(FTypeData<E>& inst, P instanceProps, A action) {};
template<> void FSDInstanceAction<ESDTypes::type1>(FTypeData<ESDTypes::type1>& inst, P instProps, A action) {
	FSDPropertyAction(inst.prop1, "prop1", instProps, action);
	FSDPropertyAction(inst.prop2, "prop2", instProps, action);
	FSDPropertyAction(inst.id, "id", instProps, action);
	if (action == A::toInstance) StaticData.type1.Add(instanceStruct.id, instanceStruct);
};
template<> void FSDInstanceAction<ESDTypes::type2>(FTypeData<ESDTypes::type2>& inst, P instProps, A action) {
	FSDPropertyAction(inst.prop1, "prop1", instProps, action);
	FSDPropertyAction(inst.prop2, "prop2", instProps, action);
	FSDPropertyAction(inst.id, "id", instProps, action);
	if (action == A::toInstance) StaticData.type2.Add(instanceStruct.id, instanceStruct);
};

struct extractedChunks {
	std::vector<std::string> chunks;
	int32 count;
};

extractedChunks extractChunks(std::string data, char delimiter = ';') {
	int32 chunkStart = 0;
	int32 chunkEnd = 0;
	extractedChunks extracted;
	extracted.count = 0;

	auto saveChunk = [&]() {
		int32 length = chunkEnd - chunkStart;
		extracted.chunks.push_back(data.substr(chunkStart, length));
		extracted.count++;
	};

	for (char c : data) {
		if (c == delimiter) {
			saveChunk();
			chunkStart = chunkEnd;
		}
		chunkEnd++;
	};

	return extracted;
};

enum class ESDSpecializations {
	createStaticData,
};

template<ESDTypes E>
void FSDSpecializedCall(ESDSpecializations spec) {
	if (spec == ESDSpecializations::createStaticData) {
		FTypeData<E> instanceStruct;
		std::vector<FSDInstanceProp> instanceProps = FDecodeInstanceData();
		FSDInstanceAction<E>(instanceStruct, instanceProps, EInstanceAction::toInstance)
		UE_LOG(LogTemp, Warning, TEXT("---------> Adding instance data to map..."));
	}
}

void FSDSpecializationJuncture(ESDTypes type, ESDSpecializations spec) {
	if (type == ESDTypes::type1) {
		FSDSpecializedCall<ESDTypes::type1>(spec);
	}
	else if (type == ESDTypes::type2) {
		FSDSpecializedCall<ESDTypes::type2>(spec);
	}
}

/**
* 
 * Data formating example:
 * 
 * 0;0,prop1Name,1347;3,prop2Name,isArray,textValue1,textValue2;\n
 * 
 * ; - delimiter
 * 
 * 0 - first zero - it's numerical value of type according to ESDTypes
 * 
 * ;0,prop1Name,1347; - this is first property
 *		0			- numerical value of type of value held according to EValueTypes
 *		prop1Name	- property name
 *		1347		- value
 * 
 * ;3,prop2Name,,textValue1,textValue2; - second property which is an array
 *		3			- numerical code for the value type
 *		prop2Name	- property name
 *		isArr		- this value is not used for anything but gives one more chunk that allows
 *						to identify series of chunks as an encoding of array-holding prop
 *		textValue1	- first of series of subsequent values the array is holding
 * 
 */
void FCreateStaticData(int typeCode) {
	if (typeCode == static_cast<int>(ESDTypes::type1)) {
		FSDSpecializationJuncture(ESDTypes::type1, ESDSpecializations::createStaticData);
	}
	else if (typeCode == static_cast<int>(ESDTypes::type2)) {
		FSDSpecializationJuncture(ESDTypes::type2, ESDSpecializations::createStaticData);
	}
}

std::vector<FSDInstanceProp> FDecodeInstanceData(std::string encodedInstance) {
	encodedInstance.erase(encodedInstance.begin());
	extractedChunks instancePropsChunks = extractChunks(encodedInstance);

	std::vector<FSDInstanceProp> instanceProps;

	for (string propChunk : instancePropsChunks.chunks) {
		FSDInstanceProp prop;

		extractedChunks propDescriptors = extractChunks(propChunk, ',');

		prop.propName = propDescriptors.chunks[1];
		prop.propValueType = static_cast<EValueTypes>(static_cast<int>(propDescriptors.chunks[0]));

		if (propDescriptors.count == 3) { // simple data
			prop.propValues.push_back(propDescriptors.chunks[2]);
			prop.propValues[1] = propDescriptors.chunks[2];
		}
		else { // array
			prop.isArray = true;
			for (std::string i = 0; (i + 3) < propDescriptors.chunks.size(); ++i) {
				prop.propValues[i] = propDescriptors.chunks[i + 3];
			}
		}

		instanceProps.push_back(prop);
	}

	return instanceProps
};

template<ESDTypes E>
std::string FEncodeInstanceData(ESDTypes instType, FTypeData<E>& inst) {
	std::vector<FSDInstanceProp> instProps;
	FSDInstanceAction<instType>(inst, instProps, EInstanceAction::toString);

	std::string encodedInstance;

	std::char propDelimiter = ';';
	std::char tokenDelimiter = ',';
	string typeCode = std::to_string(static_cast<int>(instType));
	string propName;

	encodedInstance.append(typeCode);
	encodedInstance.push_back(propDelimiter);

	for (auto prop : instProps) {
		encodedInstance.append(std::to_string(static_cast<int>(prop.propValueType)));
		encodedInstance.push_back(tokenDelimiter);
		encodedInstance.append(prop.propName);
		encodedInstance.push_back(tokenDelimiter);

		if (prop.isArray) {
			encodedInstance.append("isArray");
			encodedInstance.push_back(tokenDelimiter);

			for (auto value : prop.propValues) {
				encodedInstance.append(value);
				encodedInstance.push_back(tokenDelimiter);
			}
		}
		else {
			encodedInstance.append(prop.propValues[0]);
		}

		encodedInstance.push_back(propDelimiter);
	}

	encodedInstance.push_back('\n');
};

void SaveStaticData() {
	std::ofstream myfile;
	myfile.open("example.txt");
	std::string line;

	for (int i = ESDTypes::type1; i != ESDTypes::type2; i++)
	{
		ESDTypes currentType = static_cast<ESDTypes>(static_cast<int>(i));
		TMap<int32, FTypeData<currentType>> typeData = FSDManager::getTypeData<currentType>();

		for (auto& inst : typeData)
		{
			line.append(FEncodeInstanceData(currentType, inst));
			myfile << line << endl;
		}
	}

	myfile.close();
}

USTRUCT(BlueprintType)
struct FStaticData {
	GENERATED_BODY()

	TMap<int32, FTypeData<ESDTypes::type1>> type1;
	TMap<int32, FTypeData<ESDTypes::type2>> type2;

	bool dataIsSet;
};

void FParseRawStaticData() {
	std::ifstream infile("StaticData.txt");
	std::string line;

	while (std::getline(infile, line))
	{
		int typeCode = static_cast<int>(line[0]);
		FCreateStaticData(typeCode);
	}
}

class FSDManager {
public:
	static FStaticData StaticData;

	static bool staticDataLoaded() {
		return StaticData.dataIsSet;
	};

	template<ESDTypes E>
	static FTypeData<E>& getTypeInstanceData(int32 instanceId) {
		TMap<int32, FTypeData<E>> typeData = getTypeData<E>();
		bool hasInstance = typeData.Contains(instanceId);
		if (!hasInstance) {
			UE_LOG(LogTemp, Warning, TEXT("---------> There was no item"));
		}
		return typeData[instanceId];
	};

	template<ESDTypes E>
	static TMap<int32, FTypeData<E>> getTypeData() { return StaticData.type1; };
	template<> static TMap<int32, FTypeData<ESDTypes::type1>> getTypeData<ESDTypes::type1>() { return StaticData.type1; };
	template<> static TMap<int32, FTypeData<ESDTypes::type2>> getTypeData<ESDTypes::type2>() { return StaticData.type2; };

	static void FSetStaticData() {
		UE_LOG(LogTemp, Warning, TEXT("---------> Running FSetStaticData"));

		FParseRawStaticData();

		// type1
		//FTypeData<ESDTypes::type1> entity1type1;
		//entity1type1.id = 1;
		//entity1type1.prop1 = 11;
		//entity1type1.prop2 = 12;

		//UE_LOG(LogTemp, Warning, TEXT("---------> adding static data to map..."));

		//StaticData.type1.Add(entity1type1.id, entity1type1);

		//// type2
		//FTypeData<ESDTypes::type2> entity1type2;
		//entity1type1.id = 21;
		//entity1type1.prop1 = 21;
		//entity1type1.prop2 = 22;

		//StaticData.type2.Add(entity1type2.id, entity1type2);

		//// Wrapping up
		//StaticData.dataIsSet = true;
		////return FSDManager::StaticData;
	}
};
