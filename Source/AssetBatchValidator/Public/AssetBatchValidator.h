// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
#include "ABVTypes.h"

class FAssetBatchValidatorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
    //void ScanTextures();
    //void ScanTextures(const FName& RootPath, bool bRecursive, TArray<FABVTextureRow>& OutRows);

    void SetTargetFolder(const FName& InPath) { TargetFolder = InPath; }
    FName GetTargetFolder() const { return TargetFolder; }

    void SetRecursive(bool bIn) { bRecursiveScan = bIn; }
    bool GetRecursive() const { return bRecursiveScan; }
    void ValidateTextures(
        const FName& RootPath,
        bool bRecursive,
        int32 MaxTextureSize,
        bool bCheckPowerOfTwo,
        bool bEnableNormalRules,
        bool bEnableORMRules,
        bool bCheckMipGen,
        bool bCheckLODGroup,
        TArray<FABVTextureIssue>& OutIssues);
    // Fix: returns number of textures actually changed
    int32 FixTextures(const TArray<FABVTextureIssue>& IssuesToFix, int32 MaxTextureSize);
    
private:
    FName TargetFolder = FName("/Game");
    bool bRecursiveScan = true;
};
