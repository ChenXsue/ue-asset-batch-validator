//
//  ABVTypes.h
//  UE_EditorTools (Mac)
//
//  Created by Sue on 1/20/26.
//  Copyright 2026 Epic Games, Inc. All rights reserved.
//
#pragma once
#include "CoreMinimal.h"

enum class EABVIssueSeverity : uint8
{
    OK,
    Warning,
    Error
};

enum class EABVTextureKind : uint8
{
    Unknown,
    Color,      // BaseColor/Albedo
    Normal,
    ORM,        // Occlusion-Roughness-Metallic / Masks
    Mask        // 单独的 mask/roughness/ao 等
};

struct FABVTextureIssue
{
    FString AssetPath;
    int32 Width = 0;
    int32 Height = 0;

    EABVTextureKind Kind = EABVTextureKind::Unknown;
    EABVIssueSeverity Severity = EABVIssueSeverity::OK;

    FString Message;   // 例如 "Normal should disable sRGB"
    bool bCanFix = false;
    bool bSelected = false;
};
