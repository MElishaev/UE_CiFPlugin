# AGENTS.md

## Purpose
- This directory (`Plugins/CiF`) is an active development area.
- Prefer implementing requested changes directly in this plugin when the task touches social simulation, narrative, or plugin utilities.
- Do not avoid plugin edits by default.

## Plugin Layout
- Descriptor: `Plugins/CiF/CiF.uplugin`
- Modules:
  - `Source/CiF` (main social simulation runtime)
  - `Source/FileSystemUtilities` (file/JSON helpers)
- Key domains under `Source/CiF`:
  - Core simulation (`CiFManager`, predicates, rules, networks, SFDB)
  - Narrative (`Public/Narrative`, `Private/Narrative`)
  - Quest (`Public/CiFQuest`, `Private/CiFQuest`)
  - Demo integration (`Public/Demo`, `Private/Demo`)

## Edit Policy
- Safe and expected to modify:
  - `Plugins/CiF/Source/**`
  - `Plugins/CiF/Config/**`
  - `Plugins/CiF/README.md`
  - `Plugins/CiF/Content/**` when the task explicitly requires asset-side plugin changes
- Do not hand-edit generated outputs:
  - `Plugins/CiF/Binaries/**`
  - `Plugins/CiF/Intermediate/**`

## C++/Unreal Conventions
- Preserve UE reflection compatibility (`UCLASS`, `USTRUCT`, `UPROPERTY`, `UFUNCTION`) and generated-header ordering.
- Keep `Public/` and `Private/` boundaries aligned when adding new classes.
- Prefer forward declarations in headers; include heavy dependencies in `.cpp` where practical.
- Keep APIs backward-compatible where feasible; if breaking changes are required, document migration impact for game module/Blueprint users.
- Add comments to complex or long code blocks for reader clarity

## Dependencies And Integration
- `CiF.Build.cs` currently depends on: `Core`, `GLS`, `Json`, `JsonUtilities`, `FileSystemUtilities`, `MK_UI`, and engine modules.
- When adding dependencies, update `*.Build.cs` intentionally and call out why.
- If gameplay tags, JSON schema, or subsystem interfaces change, also note required updates in project config/Blueprints.

## Validation
- Preferred validation is compiling `CiFPrototypeEditor` after plugin edits.
- At minimum, compile-check affected modules (`CiF`, `FileSystemUtilities`) and verify no UHT/reflection errors.
- If runtime behavior is changed (rules, predicates, narrative/quest flow), include a brief manual verification checklist in the change summary.

## Collaboration Notes
- Keep changes focused to requested behavior.
- Do not revert unrelated local user changes in this plugin.
- When uncertain between game-module and plugin ownership, default to plugin implementation and ask only if architectural intent is unclear.