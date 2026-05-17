# Story Authoring (Build-Time Packs)

## Goal
Keep story authoring **structure-first** and make copy edits **data-driven**:
- change dialogue without editing C++
- validate authoring at build time (missing keys fail fast)
- keep runtime deterministic (no hot reload required)

## Obsidian Flowchart (Canonical Storyboard)
The canonical “structure view” is an Obsidian Canvas:
- `STAGE2_SEASON1_STRUCTURE.canvas`

To open it:
1. Add the repo folder to an Obsidian vault (or open the repo as a vault)
2. Ensure Obsidian core plugin **Canvas** is enabled
3. Open `STAGE2_SEASON1_STRUCTURE.canvas` from the file explorer

Note: Mermaid flowcharts in Markdown are useful snapshots, but Canvas is the real draggable storyboard.

## Week 01 Text Pack
Current pack:
- `story/content/season1/week01.toml`

Schema:
- Top-level:
  - `type` (must be `"week_text"`)
  - `pack_id` (string)
  - `node_id` (must be `"club_week_01"`)
- Repeated tables:
  - `[[scene_text]]`
    - `key` (must match `story_text_week::SceneKey` enumerator name)
    - `text` (string)
  - `[[match_start_feedback]]`
    - `match_kind` (one of: `Training`, `Official`, `Imagination1967`, `TixLunch`)
    - `line_1` (string)
    - `line_2` (string)

## Season 1 Graph Pack
Current pack:
- `story/content/season1/season1_graph.toml`

Schema:
- Top-level:
  - `type` (must be `"season_graph"`)
  - `pack_id` (string)
  - `season_id` (must be `"season1"`)
- Repeated tables:
  - `[[hub_node]]`
    - `node_id` (string; start node must be `"club_week_01"`)
    - `display_week` (int, >= 1)
    - `training_rival_id` (string; must match a `StoryRivalId` name like `"Kai"`)
    - `official_rival_id` (string; must match a `StoryRivalId` name like `"Aya"`)
    - `next_node_id` (string; empty means “end of authored content”)

## Build-Time Compilation
During `cmake --build ...`, CMake runs:
- `story_pack_compiler` (tool)
- generates `build/generated/story/story_pack_gen.cpp`
- links it as `story_pack_gen`

If the pack is missing required keys or contains unknown keys, the build fails with a file/line error.

## Editing Copy
1. Edit `story/content/season1/week01.toml`
2. Rebuild (host-safe): `cmake --build build -j2`
3. Run targeted tests: `ctest --test-dir build -R story_text_week_smoke`

## Notes
- This is intentionally build-time only right now (no runtime content IO).
- Week 01 copy is now centralized in TOML; engine logic stays in C++.
