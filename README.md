# UE Asset Batch Validator (Editor Plugin)

Production-style **Unreal Engine Editor plugin** for validating and auto-fixing assets at scale.

Assets imported into Unreal frequently suffer from issues such as incorrect texture compression or sRGB settings, oversized textures that impact performance, and inconsistent asset standards across folders and teams.

This tool demonstrates **engine-side tooling**, **batch processing**, and **artist-facing UI**, designed to automate common asset checks that are slow and error-prone when done manually.

---
## 🎬 Demo

![Preview](UE_ABV_Demo_HD.gif)

## ✨ Features


#### Editor UIs
	•	Custom Slate-based editor panel
	•	Folder selection with recursive scan
	•	Structured result table with severity color coding
	•	Row selection with checkboxes
	•	Double-click to sync asset in Content Browser
	•	In-editor success / failure notifications

#### Texture Validation
	•	Configurable Max Texture Size
	•	Power-of-Two validation
	•	Compression rule enforcement
	•	Normal → TC_Normalmap
	•	ORM / Mask → TC_Masks
	•	sRGB validation for Normal / ORM / Mask
	•	Severity classification: OK / Warning / Error

#### One-Click Fix (Key Feature)
	•	Fix Selected
	•	Fix All Failed
	•	Automatic correction of compression & sRGB settings
	•	Full Undo / Redo support
	•	Automatic refresh after fix

#### Reporting
	•	Export validation results to CSV
	•	Includes asset path, resolution, type, severity, and message

---

## 🧰 Tech Stack

	•	Unreal Engine **5.5**
	•	C++ Editor Plugin
	•	Slate UI
	•	Asset Registry
	•	ToolMenus
	•	FScopedTransaction (Undo/Redo)
	•	FSlateNotificationManager

---

## 📁 Repository Structure

```
AssetBatchValidator/
├── AssetBatchValidator.uplugin
├── Resources/
│   └── Icon128.png
├── Content/
└── Source/
    └── AssetBatchValidator/
        ├── AssetBatchValidator.Build.cs
        ├── Public/
        │   ├── AssetBatchValidator.h
        │   └── ABVTypes.h
        └── Private/
            ├── AssetBatchValidator.cpp
            ├── SAssetBatchValidatorPanel.h
            └── SAssetBatchValidatorPanel.cpp
``` 
