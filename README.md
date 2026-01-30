# UE Asset Batch Validator (Editor Plugin)

Production-style **Unreal Engine Editor plugin** for validating and auto-fixing assets at scale.

Assets imported into Unreal frequently suffer from issues such as incorrect texture compression or sRGB settings, oversized textures that impact performance, and inconsistent asset standards across folders and teams.

This tool demonstrates **engine-side tooling**, **batch processing**, and **artist-facing UI**, designed to automate common asset checks that are slow and error-prone when done manually.

---

## ✨ Features

### ✅ Implemented

#### Editor UI
- ✅ Custom **Editor panel** (Slate-based)
- ✅ Target folder display
- ✅ Recursive / non-recursive scanning
- ✅ Result table with severity and messages
- ✅ Per-row selection with checkboxes

#### Texture Validation
- ✅ Folder-based batch scan
- ✅ Max Texture Size check (configurable)
- ✅ Power-of-Two validation
- ✅ Compression rules:
  - Normal → `TC_Normalmap`
  - ORM / Mask → `TC_Masks`
- ✅ sRGB rules:
  - Normal / ORM / Mask → sRGB disabled
- ✅ Severity levels: **OK / Warning / Error**

#### One-Click Fix (Key Feature)
- ✅ Fix Selected
- ✅ Fix All Failed
- ✅ Auto-fix:
  - Compression Settings
  - sRGB flags
- ✅ Undo/Redo supported
- ✅ Auto refresh after fix

---

### 🛠️ Planned / In Progress

#### Texture Rules
- ⏳ MipGenSettings validation
- ⏳ LODGroup validation
- ⏳ Rule presets (Game / Mobile)

#### Static Mesh Validation
- ⏳ Triangle / Vertex count thresholds
- ⏳ LOD existence checks
- ⏳ Collision presence checks
- ⏳ Auto-generate simple collision (optional)

#### Visualization & UX
- ⏳ Sync asset selection with Content Browser
- ⏳ Double-click to open asset / editor
- ⏳ Preview / details panel for failed rules
- ⏳ Viewport focus / highlight (for meshes)

#### Pipeline Features
- ⏳ Export validation report (CSV / JSON)
- ⏳ Rule profiles (configurable & reusable)
- ⏳ Info / Warning / Error color coding

---

## 🧰 Tech Stack

- Unreal Engine **5.5**
- C++ Editor Plugin
- Slate UI (SListView, SHeaderRow, custom widgets)
- Asset Registry
- Editor ToolMenus
- Undo/Redo via `FScopedTransaction`

---

## 📁 Repository Structure
AssetBatchValidator/
├── AssetBatchValidator.uplugin
└── Source/
    └── AssetBatchValidator/
       ├── Public/
       │   ├── AssetBatchValidator.h
       │   └── ABVTypes.h
       └── Private/
           ├── AssetBatchValidator.cpp
           ├── SAssetBatchValidatorPanel.h
           └── SAssetBatchValidatorPanel.cpp
