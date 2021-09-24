// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "Isolation/Public/SCharacter.h"
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4883)
#endif
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeSCharacter() {}
// Cross Module References
	ISOLATION_API UClass* Z_Construct_UClass_ASCharacter_NoRegister();
	ISOLATION_API UClass* Z_Construct_UClass_ASCharacter();
	ENGINE_API UClass* Z_Construct_UClass_ACharacter();
	UPackage* Z_Construct_UPackage__Script_Isolation();
	ENGINE_API UClass* Z_Construct_UClass_USkeletalMeshComponent_NoRegister();
	ENGINE_API UClass* Z_Construct_UClass_UCameraComponent_NoRegister();
	ENGINE_API UClass* Z_Construct_UClass_USpringArmComponent_NoRegister();
// End Cross Module References
	void ASCharacter::StaticRegisterNativesASCharacter()
	{
	}
	UClass* Z_Construct_UClass_ASCharacter_NoRegister()
	{
		return ASCharacter::StaticClass();
	}
	struct Z_Construct_UClass_ASCharacter_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_MeshComp_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_MeshComp;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_CameraComp_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_CameraComp;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_SpringArmComp_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_SpringArmComp;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_ASCharacter_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_ACharacter,
		(UObject* (*)())Z_Construct_UPackage__Script_Isolation,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_ASCharacter_Statics::Class_MetaDataParams[] = {
		{ "HideCategories", "Navigation" },
		{ "IncludePath", "SCharacter.h" },
		{ "ModuleRelativePath", "Public/SCharacter.h" },
	};
#endif
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_ASCharacter_Statics::NewProp_MeshComp_MetaData[] = {
		{ "Category", "Components" },
		{ "Comment", "//Hands mesh, assignable through blueprints\n" },
		{ "EditInline", "true" },
		{ "ModuleRelativePath", "Public/SCharacter.h" },
		{ "ToolTip", "Hands mesh, assignable through blueprints" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UClass_ASCharacter_Statics::NewProp_MeshComp = { "MeshComp", nullptr, (EPropertyFlags)0x00100000010a001d, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(ASCharacter, MeshComp), Z_Construct_UClass_USkeletalMeshComponent_NoRegister, METADATA_PARAMS(Z_Construct_UClass_ASCharacter_Statics::NewProp_MeshComp_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_ASCharacter_Statics::NewProp_MeshComp_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_ASCharacter_Statics::NewProp_CameraComp_MetaData[] = {
		{ "Category", "Components" },
		{ "Comment", "//Camera Comp - component for the FPS camera\n" },
		{ "EditInline", "true" },
		{ "ModuleRelativePath", "Public/SCharacter.h" },
		{ "ToolTip", "Camera Comp - component for the FPS camera" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UClass_ASCharacter_Statics::NewProp_CameraComp = { "CameraComp", nullptr, (EPropertyFlags)0x00100000010a001d, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(ASCharacter, CameraComp), Z_Construct_UClass_UCameraComponent_NoRegister, METADATA_PARAMS(Z_Construct_UClass_ASCharacter_Statics::NewProp_CameraComp_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_ASCharacter_Statics::NewProp_CameraComp_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_ASCharacter_Statics::NewProp_SpringArmComp_MetaData[] = {
		{ "Category", "Components" },
		{ "Comment", "//Spring Arm Comp - component for the spring arm, which is required to enable 'use control rotation'\n" },
		{ "EditInline", "true" },
		{ "ModuleRelativePath", "Public/SCharacter.h" },
		{ "ToolTip", "Spring Arm Comp - component for the spring arm, which is required to enable 'use control rotation'" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UClass_ASCharacter_Statics::NewProp_SpringArmComp = { "SpringArmComp", nullptr, (EPropertyFlags)0x00100000010a001d, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(ASCharacter, SpringArmComp), Z_Construct_UClass_USpringArmComponent_NoRegister, METADATA_PARAMS(Z_Construct_UClass_ASCharacter_Statics::NewProp_SpringArmComp_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_ASCharacter_Statics::NewProp_SpringArmComp_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_ASCharacter_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_ASCharacter_Statics::NewProp_MeshComp,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_ASCharacter_Statics::NewProp_CameraComp,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_ASCharacter_Statics::NewProp_SpringArmComp,
	};
	const FCppClassTypeInfoStatic Z_Construct_UClass_ASCharacter_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<ASCharacter>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_ASCharacter_Statics::ClassParams = {
		&ASCharacter::StaticClass,
		"Game",
		&StaticCppClassTypeInfo,
		DependentSingletons,
		nullptr,
		Z_Construct_UClass_ASCharacter_Statics::PropPointers,
		nullptr,
		UE_ARRAY_COUNT(DependentSingletons),
		0,
		UE_ARRAY_COUNT(Z_Construct_UClass_ASCharacter_Statics::PropPointers),
		0,
		0x009000A4u,
		METADATA_PARAMS(Z_Construct_UClass_ASCharacter_Statics::Class_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UClass_ASCharacter_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_ASCharacter()
	{
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_ASCharacter_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(ASCharacter, 4167558528);
	template<> ISOLATION_API UClass* StaticClass<ASCharacter>()
	{
		return ASCharacter::StaticClass();
	}
	static FCompiledInDefer Z_CompiledInDefer_UClass_ASCharacter(Z_Construct_UClass_ASCharacter, &ASCharacter::StaticClass, TEXT("/Script/Isolation"), TEXT("ASCharacter"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(ASCharacter);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
#ifdef _MSC_VER
#pragma warning (pop)
#endif
