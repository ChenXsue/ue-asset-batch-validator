//
//  SAssetBatchValidatorPanel.cpp
//  UE_EditorTools (Mac)
//
//  Created by Sue on 1/17/26.
//  Copyright 2026 Epic Games, Inc. All rights reserved.
//

#include "SAssetBatchValidatorPanel.h"

#include "CoreMinimal.h"

#include "Modules/ModuleManager.h"

#include "AssetBatchValidator.h" // defines EABVTextureKind, EABVIssueSeverity, FABVTextureIssue

#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Views/STableRow.h"

static FString KindToString(EABVTextureKind K)
{
    switch (K)
    {
    case EABVTextureKind::Color:  return TEXT("Color");
    case EABVTextureKind::Normal: return TEXT("Normal");
    case EABVTextureKind::ORM:    return TEXT("ORM");
    case EABVTextureKind::Mask:   return TEXT("Mask");
    default:                      return TEXT("Unknown");
    }
}

static FString SeverityToString(EABVIssueSeverity S)
{
    switch (S)
    {
    case EABVIssueSeverity::OK:      return TEXT("OK");
    case EABVIssueSeverity::Warning: return TEXT("Warning");
    case EABVIssueSeverity::Error:   return TEXT("Error");
    default:                          return TEXT("OK");
    }
}

FReply SAssetBatchValidatorPanel::OnUseSelectedFolderClicked()
{
    FContentBrowserModule& CB =
        FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

    TArray<FString> Paths;
    CB.Get().GetSelectedPathViewFolders(Paths);

    if (Paths.Num() > 0)
    {
        FString P = Paths[0];
        // /All/Game/... -> /Game/...
        P.ReplaceInline(TEXT("/All/Game"), TEXT("/Game"));
        TargetFolder = FName(*P);

        UE_LOG(LogTemp, Display, TEXT("ABV: TargetFolder set to %s"), *TargetFolder.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("ABV: No folder selected in Content Browser."));
    }

    return FReply::Handled();
}

FReply SAssetBatchValidatorPanel::OnScanTexturesClicked()
{
    FAssetBatchValidatorModule& Mod =
        FModuleManager::LoadModuleChecked<FAssetBatchValidatorModule>("AssetBatchValidator");

    TArray<FABVTextureIssue> Issues;

    Mod.ValidateTextures(
        TargetFolder,
        bRecursive,
        MaxTextureSize,
        bCheckPowerOfTwo,
        bEnableNormalRules,
        bEnableORMRules,
        bCheckMipGen,
        bCheckLODGroup,
        Issues
    );

    TextureItems.Reset();
    for (const FABVTextureIssue& It : Issues)
    {
        TextureItems.Add(MakeShared<FABVTextureIssue>(It));
    }

    if (TextureListView.IsValid())
    {
        TextureListView->RequestListRefresh();
    }

    return FReply::Handled();
}

FReply SAssetBatchValidatorPanel::OnFixSelectedClicked()
{
    TArray<FABVTextureIssue> ToFix;

    for (const TSharedPtr<FABVTextureIssue>& Item : TextureItems)
    {
        if (!Item.IsValid()) continue;
        if (!Item->bSelected) continue;
        if (Item->Severity == EABVIssueSeverity::OK) continue;
        if (!Item->bCanFix) continue;

        ToFix.Add(*Item);
    }

    if (ToFix.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("ABV: No checked rows to fix."));
        return FReply::Handled();
    }

    FAssetBatchValidatorModule& Mod =
        FModuleManager::LoadModuleChecked<FAssetBatchValidatorModule>("AssetBatchValidator");

    const int32 Fixed = Mod.FixTextures(ToFix, MaxTextureSize);
    UE_LOG(LogTemp, Display, TEXT("ABV: Fixed %d checked textures."), Fixed);

    OnScanTexturesClicked();
    return FReply::Handled();
}

FReply SAssetBatchValidatorPanel::OnFixAllFailedClicked()
{
    TArray<FABVTextureIssue> ToFix;
    for (const auto& Item : TextureItems)
    {
        if (!Item.IsValid()) continue;
        if (Item->Severity == EABVIssueSeverity::OK) continue;
        if (!Item->bCanFix) continue;
        ToFix.Add(*Item);
    }

    FAssetBatchValidatorModule& Mod =
        FModuleManager::LoadModuleChecked<FAssetBatchValidatorModule>("AssetBatchValidator");

    const int32 Fixed = Mod.FixTextures(ToFix, MaxTextureSize);
    UE_LOG(LogTemp, Display, TEXT("ABV: Fixed %d textures (All Failed)."), Fixed);

    return OnScanTexturesClicked();
}

TSharedRef<ITableRow> SAssetBatchValidatorPanel::OnGenerateRow(
    TSharedPtr<FABVTextureIssue> Item,
    const TSharedRef<STableViewBase>& OwnerTable)
{
    const FString SizeText = FString::Printf(TEXT("%dx%d"), Item->Width, Item->Height);

    return SNew(STableRow<TSharedPtr<FABVTextureIssue>>, OwnerTable)
    [
        SNew(SHorizontalBox)

        // 0) Checkbox 列（最左边）
        + SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        .Padding(6, 2)
        [
            SNew(SCheckBox)
            .IsChecked_Lambda([Item]()
            {
                if (!Item.IsValid()) return ECheckBoxState::Unchecked;
                return Item->bSelected ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
            })
            .OnCheckStateChanged_Lambda([Item](ECheckBoxState NewState)
            {
                if (Item.IsValid())
                {
                    Item->bSelected = (NewState == ECheckBoxState::Checked);
                }
            })
        ]

        // 1) Asset（只显示文件名）
        + SHorizontalBox::Slot().FillWidth(0.40f).Padding(6,2)
        [
            SNew(STextBlock).Text(FText::FromString(FPackageName::GetShortName(Item->AssetPath)))
        ]

        // 2) Size
        + SHorizontalBox::Slot().FillWidth(0.12f).Padding(6,2)
        [
            SNew(STextBlock).Text(FText::FromString(SizeText))
        ]

        // 3) Type
        + SHorizontalBox::Slot().FillWidth(0.10f).Padding(6,2)
        [
            SNew(STextBlock).Text(FText::FromString(KindToString(Item->Kind)))
        ]

        // 4) Result
        + SHorizontalBox::Slot().FillWidth(0.12f).Padding(6,2)
        [
            SNew(STextBlock).Text(FText::FromString(SeverityToString(Item->Severity)))
        ]

        // 5) Message
        + SHorizontalBox::Slot().FillWidth(0.26f).Padding(6,2)
        [
            SNew(STextBlock).Text(FText::FromString(Item->Message))
        ]
    ];
}
void SAssetBatchValidatorPanel::OnRecursiveChanged(ECheckBoxState NewState)
{
    bRecursive = (NewState == ECheckBoxState::Checked);
}

ECheckBoxState SAssetBatchValidatorPanel::GetRecursiveState() const
{
    return bRecursive ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

FText SAssetBatchValidatorPanel::GetTargetFolderText() const
{
    return FText::FromString(TargetFolder.ToString());
}

void SAssetBatchValidatorPanel::SetAllSelected(bool bInSelected)
{
    for (const TSharedPtr<FABVTextureIssue>& Item : TextureItems)
    {
        if (Item.IsValid())
        {
            Item->bSelected = bInSelected;
        }
    }

    if (TextureListView.IsValid())
    {
        TextureListView->RequestListRefresh();
    }
}

FReply SAssetBatchValidatorPanel::OnSelectAllClicked()
{
    SetAllSelected(true);
    return FReply::Handled();
}

FReply SAssetBatchValidatorPanel::OnClearSelectionClicked()
{
    SetAllSelected(false);
    return FReply::Handled();
}

void SAssetBatchValidatorPanel::Construct(const FArguments& InArgs)
{
    ChildSlot
    [
        SNew(SVerticalBox)
        
        + SVerticalBox::Slot().AutoHeight().Padding(8)
        [
            SNew(STextBlock).Text(FText::FromString(TEXT("Asset Batch Validator")))
        ]
        
        + SVerticalBox::Slot().AutoHeight().Padding(8)
        [
            SNew(STextBlock)
                .Text_Lambda([this]()
                             {
                                 return FText::FromString(FString::Printf(TEXT("Target Folder: %s"), *TargetFolder.ToString()));
                             })
        ]
        
        + SVerticalBox::Slot().AutoHeight().Padding(8)
        [
            SNew(SCheckBox)
                .IsChecked(this, &SAssetBatchValidatorPanel::GetRecursiveState)
                .OnCheckStateChanged(this, &SAssetBatchValidatorPanel::OnRecursiveChanged)
            [
                SNew(STextBlock).Text(FText::FromString(TEXT("Recursive")))
            ]
        ]
        
        + SVerticalBox::Slot().AutoHeight().Padding(8)
        [
            SNew(SButton)
                .Text(FText::FromString(TEXT("Use Selected Folder")))
                .OnClicked(this, &SAssetBatchValidatorPanel::OnUseSelectedFolderClicked)
        ]
        
        + SVerticalBox::Slot().AutoHeight().Padding(8)
        [
            SNew(SButton)
                .Text(FText::FromString(TEXT("Scan Textures")))
                .OnClicked(this, &SAssetBatchValidatorPanel::OnScanTexturesClicked)
        ]
        
        + SVerticalBox::Slot().AutoHeight().Padding(8)
        [
            SNew(SHorizontalBox)

            + SHorizontalBox::Slot().AutoWidth().Padding(0,0,8,0)
            [
                SNew(SButton)
                .Text(FText::FromString(TEXT("Fix Selected")))
                .OnClicked(this, &SAssetBatchValidatorPanel::OnFixSelectedClicked)
            ]

            + SHorizontalBox::Slot().AutoWidth()
            [
                SNew(SButton)
                .Text(FText::FromString(TEXT("Fix All Failed")))
                .OnClicked(this, &SAssetBatchValidatorPanel::OnFixAllFailedClicked)
            ]
            
            + SHorizontalBox::Slot().AutoWidth().Padding(0,0,8,0)
            [
                SNew(SButton)
                .Text(FText::FromString(TEXT("Select All")))
                .OnClicked(this, &SAssetBatchValidatorPanel::OnSelectAllClicked)
            ]

            + SHorizontalBox::Slot().AutoWidth()
            [
                SNew(SButton)
                .Text(FText::FromString(TEXT("Clear Selection")))
                .OnClicked(this, &SAssetBatchValidatorPanel::OnClearSelectionClicked)
            ]
        ]
        

        
        // Result list
        + SVerticalBox::Slot().FillHeight(1.0f).Padding(8)
        [
            SAssignNew(TextureListView, SListView<TSharedPtr<FABVTextureIssue>>)
                .ListItemsSource(&TextureItems)
                .OnGenerateRow(this, &SAssetBatchValidatorPanel::OnGenerateRow)
                .HeaderRow(
                    SNew(SHeaderRow)

                    + SHeaderRow::Column("AssetPath")
                    .DefaultLabel(FText::FromString("Asset"))
                    .FillWidth(0.45f)

                    + SHeaderRow::Column("Size")
                    .DefaultLabel(FText::FromString("Size"))
                    .FillWidth(0.10f)

                    + SHeaderRow::Column("Kind")
                    .DefaultLabel(FText::FromString("Type"))
                    .FillWidth(0.10f)

                    + SHeaderRow::Column("Severity")
                    .DefaultLabel(FText::FromString("Result"))
                    .FillWidth(0.10f)

                    + SHeaderRow::Column("Message")
                    .DefaultLabel(FText::FromString("Message"))
                    .FillWidth(0.25f))
        ]
    ];
    
}
