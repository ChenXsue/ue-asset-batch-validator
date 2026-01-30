//
//  SAssetBatchValidatorPanel.h
//  UE_EditorTools (Mac)
//
//  Created by Sue on 1/17/26.
//  Copyright 2026 Epic Games, Inc. All rights reserved.
//

#pragma once
#include "Widgets/SBoxPanel.h"
#include "ABVTypes.h"
template<typename ItemType> class SListView;

class SAssetBatchValidatorPanel : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SAssetBatchValidatorPanel) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

private:
    // UI actions
    FReply OnUseSelectedFolderClicked();
    FReply OnScanTexturesClicked();
    FReply OnFixSelectedClicked();
    FReply OnFixAllFailedClicked();
    FReply OnSelectAllClicked();
    FReply OnClearSelectionClicked();

    // Text / checkbox
    FText GetTargetFolderText() const;
    void OnRecursiveChanged(ECheckBoxState NewState);
    void SetAllSelected(bool bInSelected);
    ECheckBoxState GetRecursiveState() const;


private:
    bool bRecursive = true;
    FName TargetFolder = FName("/Game");
    int32 MaxTextureSize = 2048;
    bool bCheckPowerOfTwo = true;

    bool bEnableNormalRules = true;
    bool bEnableORMRules = true;

    bool bCheckMipGen = false;     
    bool bCheckLODGroup = false;
    
    // List data (ListView 用 SharedPtr)
    TSharedRef<ITableRow> OnGenerateRow(TSharedPtr<FABVTextureIssue> Item, const TSharedRef<STableViewBase>& OwnerTable);

    TArray<TSharedPtr<FABVTextureIssue>> TextureItems;
    TSharedPtr<SListView<TSharedPtr<FABVTextureIssue>>> TextureListView;
};
