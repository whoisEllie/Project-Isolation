// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
#ifdef ISOLATION_SCharacter_generated_h
#error "SCharacter.generated.h already included, missing '#pragma once' in SCharacter.h"
#endif
#define ISOLATION_SCharacter_generated_h

#define Isolation_Source_Isolation_Public_SCharacter_h_17_SPARSE_DATA
#define Isolation_Source_Isolation_Public_SCharacter_h_17_RPC_WRAPPERS
#define Isolation_Source_Isolation_Public_SCharacter_h_17_RPC_WRAPPERS_NO_PURE_DECLS
#define Isolation_Source_Isolation_Public_SCharacter_h_17_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesASCharacter(); \
	friend struct Z_Construct_UClass_ASCharacter_Statics; \
public: \
	DECLARE_CLASS(ASCharacter, ACharacter, COMPILED_IN_FLAGS(0 | CLASS_Config), CASTCLASS_None, TEXT("/Script/Isolation"), NO_API) \
	DECLARE_SERIALIZER(ASCharacter)


#define Isolation_Source_Isolation_Public_SCharacter_h_17_INCLASS \
private: \
	static void StaticRegisterNativesASCharacter(); \
	friend struct Z_Construct_UClass_ASCharacter_Statics; \
public: \
	DECLARE_CLASS(ASCharacter, ACharacter, COMPILED_IN_FLAGS(0 | CLASS_Config), CASTCLASS_None, TEXT("/Script/Isolation"), NO_API) \
	DECLARE_SERIALIZER(ASCharacter)


#define Isolation_Source_Isolation_Public_SCharacter_h_17_STANDARD_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API ASCharacter(const FObjectInitializer& ObjectInitializer); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(ASCharacter) \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, ASCharacter); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(ASCharacter); \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API ASCharacter(ASCharacter&&); \
	NO_API ASCharacter(const ASCharacter&); \
public:


#define Isolation_Source_Isolation_Public_SCharacter_h_17_ENHANCED_CONSTRUCTORS \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API ASCharacter(ASCharacter&&); \
	NO_API ASCharacter(const ASCharacter&); \
public: \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, ASCharacter); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(ASCharacter); \
	DEFINE_DEFAULT_CONSTRUCTOR_CALL(ASCharacter)


#define Isolation_Source_Isolation_Public_SCharacter_h_17_PRIVATE_PROPERTY_OFFSET
#define Isolation_Source_Isolation_Public_SCharacter_h_14_PROLOG
#define Isolation_Source_Isolation_Public_SCharacter_h_17_GENERATED_BODY_LEGACY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	Isolation_Source_Isolation_Public_SCharacter_h_17_PRIVATE_PROPERTY_OFFSET \
	Isolation_Source_Isolation_Public_SCharacter_h_17_SPARSE_DATA \
	Isolation_Source_Isolation_Public_SCharacter_h_17_RPC_WRAPPERS \
	Isolation_Source_Isolation_Public_SCharacter_h_17_INCLASS \
	Isolation_Source_Isolation_Public_SCharacter_h_17_STANDARD_CONSTRUCTORS \
public: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


#define Isolation_Source_Isolation_Public_SCharacter_h_17_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	Isolation_Source_Isolation_Public_SCharacter_h_17_PRIVATE_PROPERTY_OFFSET \
	Isolation_Source_Isolation_Public_SCharacter_h_17_SPARSE_DATA \
	Isolation_Source_Isolation_Public_SCharacter_h_17_RPC_WRAPPERS_NO_PURE_DECLS \
	Isolation_Source_Isolation_Public_SCharacter_h_17_INCLASS_NO_PURE_DECLS \
	Isolation_Source_Isolation_Public_SCharacter_h_17_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


template<> ISOLATION_API UClass* StaticClass<class ASCharacter>();

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID Isolation_Source_Isolation_Public_SCharacter_h


PRAGMA_ENABLE_DEPRECATION_WARNINGS
