// Copyright Epic Games, Inc. All Rights Reserved.

#include "AssetBatchValidator.h"
#include "ToolMenus.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/Texture2D.h"
#include "SAssetBatchValidatorPanel.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/Docking/TabManager.h"
#include "ABVTypes.h"
#include "ScopedTransaction.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/Texture2D.h"

#define LOCTEXT_NAMESPACE "FAssetBatchValidatorModule"
static bool ABV_IsPowerOfTwo(int32 X)
{
    return X > 0 && ( (X & (X - 1)) == 0 );
}

static bool ABV_HasToken(const FString& Name, const FString& Token)
{
    const FString T = TEXT("_") + Token;

    int32 Index = Name.Find(T);
    if (Index == INDEX_NONE)
        return false;

    const int32 End = Index + T.Len();
    if (End >= Name.Len())
        return true;

    const TCHAR Next = Name[End];
    return Next == TEXT('_') || Next == TEXT('.');
}

static EABVTextureKind ABV_GuessKindFromName(const FString& ObjectPath)
{
    const FString Name =
        FPaths::GetBaseFilename(ObjectPath).ToLower();

    // ---------- Normal ----------
    if (ABV_HasToken(Name, TEXT("n")) ||
        Name.Contains(TEXT("normal")))
    {
        return EABVTextureKind::Normal;
    }

    // ---------- ORM ----------
    if (Name.Contains(TEXT("orm")) ||
        Name.Contains(TEXT("rma")) ||
        Name.Contains(TEXT("mra")))
    {
        return EABVTextureKind::ORM;
    }

    // ---------- Mask ----------
    if (Name.Contains(TEXT("mask")) ||
        Name.Contains(TEXT("opacity")) ||
        Name.Contains(TEXT("ao")))
    {
        return EABVTextureKind::Mask;
    }

    // ---------- Color ----------
    return EABVTextureKind::Color;
}

static void ABV_AddIssue(
    TArray<FABVTextureIssue>& Out,
    const FString& AssetPath,
    int32 W, int32 H,
    EABVTextureKind Kind,
    EABVIssueSeverity Sev,
    const FString& Msg,
    bool bCanFix)
{
    FABVTextureIssue I;
    I.AssetPath = AssetPath;
    I.Width = W;
    I.Height = H;
    I.Kind = Kind;
    I.Severity = Sev;
    I.Message = Msg;
    I.bCanFix = bCanFix;
    Out.Add(MoveTemp(I));
}
static const FName ABV_TabName("AssetBatchValidatorTab");
/*
void FAssetBatchValidatorModule::ScanTextures(const FName& RootPath, bool bRecursive, TArray<FABVTextureRow>& OutRows)
{
    OutRows.Reset();

    IAssetRegistry& AssetRegistry =
        FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();

    FARFilter Filter;
    Filter.PackagePaths.Add(RootPath);
    Filter.ClassPaths.Add(UTexture2D::StaticClass()->GetClassPathName());
    Filter.bRecursivePaths = bRecursive;

    TArray<FAssetData> Assets;
    AssetRegistry.GetAssets(Filter, Assets);

    for (const FAssetData& AD : Assets)
    {
        UTexture2D* Tex = Cast<UTexture2D>(AD.GetAsset());
        if (!Tex) continue;

        FABVTextureRow Row;
        Row.AssetPath = AD.GetObjectPathString();
        Row.Width = Tex->GetSizeX();
        Row.Height = Tex->GetSizeY();
        Row.bOkSquare = (Row.Width == Row.Height);

        OutRows.Add(MoveTemp(Row));
    }
}
 */
void FAssetBatchValidatorModule::ValidateTextures(
    const FName& RootPath,
    bool bRecursive,
    int32 MaxTextureSize,
    bool bCheckPowerOfTwo,
    bool bEnableNormalRules,
    bool bEnableORMRules,
    bool bCheckMipGen,
    bool bCheckLODGroup,
    TArray<FABVTextureIssue>& OutIssues)
{
    OutIssues.Reset();

    IAssetRegistry& AssetRegistry =
        FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();

    FARFilter Filter;
    Filter.PackagePaths.Add(RootPath);
    Filter.ClassPaths.Add(UTexture2D::StaticClass()->GetClassPathName());
    Filter.bRecursivePaths = bRecursive;

    TArray<FAssetData> Assets;
    AssetRegistry.GetAssets(Filter, Assets);

    UE_LOG(LogTemp, Display, TEXT("=== ABV: Validate Textures in %s (%d found) ==="),
        *RootPath.ToString(), Assets.Num());

    for (const FAssetData& AD : Assets)
    {
        UTexture2D* Tex = Cast<UTexture2D>(AD.GetAsset());
        if (!Tex) continue;

        const FString AssetPath = AD.GetObjectPathString();
        const int32 W = Tex->GetSizeX();
        const int32 H = Tex->GetSizeY();

        const EABVTextureKind Kind = ABV_GuessKindFromName(AssetPath);

        // 对当前贴图只生成 1 行
        EABVIssueSeverity FinalSev = EABVIssueSeverity::OK;
        TArray<FString> Messages;

        auto AddRule = [&](EABVIssueSeverity Sev, const FString& Msg)
        {
            if (static_cast<int32>(Sev) > static_cast<int32>(FinalSev))
            {
                FinalSev = Sev;
            }
            Messages.Add(Msg);
        };

        // Rule 1: Max Texture Size
        if (MaxTextureSize > 0 && (W > MaxTextureSize || H > MaxTextureSize))
        {
            AddRule(EABVIssueSeverity::Error,
                FString::Printf(TEXT("Exceeds MaxSize (%d)"), MaxTextureSize));
        }

        // Rule 2: Power of Two
        if (bCheckPowerOfTwo)
        {
            const bool bPOT = ABV_IsPowerOfTwo(W) && ABV_IsPowerOfTwo(H);
            if (!bPOT)
            {
                AddRule(EABVIssueSeverity::Warning, TEXT("Not Power-of-Two"));
            }
        }

        // Normal rules
        if (Kind == EABVTextureKind::Normal && bEnableNormalRules)
        {
            if (Tex->SRGB)
                AddRule(EABVIssueSeverity::Error, TEXT("Normal map should disable sRGB"));

            if (Tex->CompressionSettings != TC_Normalmap)
                AddRule(EABVIssueSeverity::Warning, TEXT("Normal should use TC_Normalmap"));
        }

        // ORM/Mask rules
        if ((Kind == EABVTextureKind::ORM || Kind == EABVTextureKind::Mask) && bEnableORMRules)
        {
            if (Tex->SRGB)
                AddRule(EABVIssueSeverity::Error, TEXT("ORM/Mask should disable sRGB"));

            if (Tex->CompressionSettings != TC_Masks)
                AddRule(EABVIssueSeverity::Warning, TEXT("ORM/Mask should use TC_Masks"));
        }

        // -------------------------
        // Optional: MipGen / LODGroup (先占位)
        // -------------------------
        if (bCheckMipGen)
        {
            // 你之后可以在这里检查 Tex->MipGenSettings
            // ABV_AddIssue(..., TEXT("MipGenSettings not expected ..."));
        }

        if (bCheckLODGroup)
        {
            // 你之后可以在这里检查 Tex->LODGroup
            // ABV_AddIssue(..., TEXT("LODGroup not expected ..."));
        }
        
        const FString FinalMsg = (Messages.Num() == 0)
            ? TEXT("OK")
            : FString::Join(Messages, TEXT(" | "));

        const bool bCanFix =
            (Kind == EABVTextureKind::Normal
          || Kind == EABVTextureKind::ORM
          || Kind == EABVTextureKind::Mask);
        
        ABV_AddIssue(OutIssues, AssetPath, W, H, Kind, FinalSev, FinalMsg, bCanFix);
    }

}

int32 FAssetBatchValidatorModule::FixTextures(const TArray<FABVTextureIssue>& IssuesToFix, int32 MaxTextureSize)
{
    if (IssuesToFix.Num() == 0) return 0;

    const FScopedTransaction Tx(NSLOCTEXT("AssetBatchValidator", "FixTextures", "ABV Fix Textures"));

    int32 FixedCount = 0;

    for (const FABVTextureIssue& Issue : IssuesToFix)
    {
        if (!Issue.bCanFix) continue;
        if (Issue.Severity == EABVIssueSeverity::OK) continue;

        UTexture2D* Tex = LoadObject<UTexture2D>(nullptr, *Issue.AssetPath);
        if (!Tex) continue;

        bool bChanged = false;
        Tex->Modify(); // support Undo/Redo

        // 1) Normal rules
        if (Issue.Kind == EABVTextureKind::Normal)
        {
            if (Tex->CompressionSettings != TC_Normalmap) { Tex->CompressionSettings = TC_Normalmap; bChanged = true; }
            if (Tex->SRGB != false) { Tex->SRGB = false; bChanged = true; }
        }

        // 2) ORM/Mask rules
        if (Issue.Kind == EABVTextureKind::ORM || Issue.Kind == EABVTextureKind::Mask)
        {
            if (Tex->CompressionSettings != TC_Masks) { Tex->CompressionSettings = TC_Masks; bChanged = true; }
            if (Tex->SRGB != false) { Tex->SRGB = false; bChanged = true; }
        }

        // 3) Max size rule (Optional)
        if (MaxTextureSize > 0)
        {
            const int32 W = Tex->GetSizeX();
            const int32 H = Tex->GetSizeY();
            if (FMath::Max(W, H) > MaxTextureSize)
            {
                if (Tex->MaxTextureSize != MaxTextureSize)
                {
                    Tex->MaxTextureSize = MaxTextureSize;
                    bChanged = true;
                }
            }
        }

        if (bChanged)
        {
            Tex->PostEditChange();   // update assets status
            Tex->MarkPackageDirty(); // remind to save
            FixedCount++;
        }
    }

    return FixedCount;
}

void FAssetBatchValidatorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
        ABV_TabName,
        FOnSpawnTab::CreateLambda([](const FSpawnTabArgs& Args)
        {
            return SNew(SDockTab)
                .TabRole(ETabRole::NomadTab)
                [
                    SNew(SAssetBatchValidatorPanel)
                ];
        })
    )
    .SetDisplayName(FText::FromString(TEXT("Asset Batch Validator")))
    .SetMenuType(ETabSpawnerMenuType::Hidden);
    
    if (!UToolMenus::IsToolMenuUIEnabled())
    {
        return;
    }
    
    
    UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateLambda([]()
    {
        UToolMenu* ToolsMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");
        FToolMenuSection& Section = ToolsMenu->FindOrAddSection("AssetBatchValidator");

        Section.AddMenuEntry(
            "ABV_OpenPanel",
            LOCTEXT("ABV_OpenPanel_Label", "Asset Batch Validator: Open Panel"),
            LOCTEXT("ABV_OpenPanel_Tooltip", "Open the Asset Batch Validator panel."),
            FSlateIcon(),
            FUIAction(FExecuteAction::CreateLambda([]()
            {
                FGlobalTabmanager::Get()->TryInvokeTab(ABV_TabName);
            }))
        );
    }));
}

void FAssetBatchValidatorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
    FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(ABV_TabName);

    if (UToolMenus::IsToolMenuUIEnabled())
    {
        UToolMenus::UnregisterOwner(this);
    }
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAssetBatchValidatorModule, AssetBatchValidator)
