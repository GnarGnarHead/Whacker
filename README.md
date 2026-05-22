# Whacker

Whacker is a fast, spin-heavy Pong game wrapped in a coming-of-age sports drama.

You train, develop a playstyle, fight rivals, and try to survive an opponent that wants to make every return miserable. The ball curves, kicks, accelerates, and turns simple returns into pressure. The story layer is built around that pressure: training changes how you play, matches change how people read you, and rivals respond to the identity you build at the table.

## Screenshots

These captures reflect the current game more honestly than the older root-level development shots.

<p align="center">
  <img src="screenshots%20final/Screenshot_20260423_151917.png" alt="Whacker main menu" width="49%">
  <img src="screenshots%20final/Screenshot_20260423_151941.png" alt="Whacker match setup screen" width="49%">
</p>
<p align="center">
  <img src="screenshots%20final/Screenshot_20260423_152029.png" alt="Whacker story choice scene" width="49%">
  <img src="screenshots%20final/Screenshot_20260423_155335.png" alt="Whacker story hub screen" width="49%">
</p>

## What It Is

- A playable low-resolution Pong variant with persistent spin, speed escalation, and continuous paddle aiming.
- A deterministic simulation-first game: fixed timestep, reproducible match behavior, and AI that plays under the same mechanical rules as the player.
- A small campaign structure with training, rivals, story scenes, progression, match tags, and style expression.
- A public source-available workbench repo. Code, docs, tests, authored story content, art, studies, temporary plans, and archived development conversations are intentionally visible for noncommercial use under the project license.

## Current State

The game is playable now. Human and AI matches, menu flow, HUD, pause flow, story mode, presets, paddle tuning, save flow, and smoke tests are in place.

This is still a raw development repository, not a polished packaged release. The current screenshots live in `screenshots final/`. Older root-level `Screenshot_*.png` files remain as archived development captures.

## Build

Requirements:

- CMake 3.20+
- A C++20 compiler
- SDL2 for the playable app window, input, controllers, and audio
- OpenGL for rendering
- Optional: libpng for PNG-backed story portraits and menu stickers

```bash
cmake -S . -B build
cmake --build build -j2
```

If your machine is under memory pressure, use:

```bash
cmake --build build -j1
```

`WHACKER_BUILD_APP=ON` requires SDL2. For a headless/test-only build on a machine without SDL2, configure with:

```bash
cmake -S . -B build -DWHACKER_BUILD_APP=OFF
```

## Run

```bash
./build/whacker
```

At startup the game loads tuning values from `config/default.json` when available. Menu defaults are committed in `config/menu_defaults.cfg`; local runtime preferences are written to ignored files under `config/`.

## Controls

- Menu navigation: `Up` / `Down` / `Left` / `Right`
- Menu select/start: `Enter`
- Toggle menu in-game: `M`
- Left paddle: `W` / `S`
- Right paddle: `Up` / `Down`
- `Esc`: pause while playing, resume or cancel while paused, exit from the main menu

## Test

```bash
ctest --test-dir build --output-on-failure
```

For a narrower content-authoring check:

```bash
ctest --test-dir build --output-on-failure -R story_pack_compiler
```

## Current Tool Targets

`story_pack_compiler` is used during the build to validate and compile authored TOML story packs into generated C++.

```bash
./build/story_pack_compiler \
  --validate-only \
  --input story/content/season1/week01.toml \
  --input story/content/season1/season1_graph.toml \
  --input story/content/ui/menu_stickers.toml
```

The build also produces `play_control_duel`, a deterministic AI-control duel harness for comparing skill triplets.

```bash
./build/play_control_duel --left 0.12,0.12,0.12 --right 0.56,0.56,0.56 --games 20 --trace-hash
```

Skill triplets are `edge,power,spin`. Each value is in `[0,1]`, and the total budget must be `<= 1.70`.

## Repo Map

- `app/`: runtime loop, menus, rendering, story flow, match flow, AI integration, and saves.
- `sim/` and `include/sim/`: deterministic ball, paddle, spin, collision, and config logic.
- `progression/` and `include/progression/`: skills, style, tags, and reputation systems.
- `input/` and `include/input/`: keyboard, mouse, gamepad, and serial input adapters.
- `story/`: authored season content, portraits, stickers, character docs, and supporting material.
- `tests/`: smoke coverage for runtime, story, AI, progression, layout, content validation, and determinism.
- `tools/`: story-pack compiler, duel harness, sticker tooling, and local verification scripts.
- `docs/`: focused authoring and architecture notes.
- Root markdown files: vision docs, studies, locks, plans, archives, and process material.

## Under The Hood

Whacker is built in C++20 with CMake, SDL2, and OpenGL.

The simulation is fixed-timestep and deterministic. The AI stack uses forward simulation, reachability checks, pressure scoring, and style-specific planning rather than neural-net behavior. Story content is authored as data and validated at build time so runtime behavior stays reproducible.

## Good Starting Points

- `VISION.md`: original mechanical and AI direction.
- `STAGE2_LOCK.md`: current campaign/story/progression contract.
- `AI_SCORECARD.md`: how AI tuning is evaluated.
- `docs/story_authoring.md`: build-time story pack workflow.

Then move into:

- `app/`
- `sim/`
- `progression/`
- `story/`

## Public Repo Notes

This repository intentionally keeps process material visible: archived conversations, temporary plans, studies, old screenshots, story locks, and historical notes. Treat those files as development record unless a document explicitly says it is canonical.

The current playable surface is represented by the build, tests, `README.md`, `VISION.md`, `STAGE2_LOCK.md`, and the current screenshots in `screenshots final/`.

## License

Copyright (c) 2026 `GNARGNARHEAD`. All rights reserved except for noncommercial, educational, research, personal, and other uses expressly permitted by the PolyForm Noncommercial License 1.0.0 (`PolyForm-Noncommercial-1.0.0`). See `LICENSE`.

Commercial use outside those terms requires separate written permission from `GNARGNARHEAD`.

## Credits

- Created by `GNARGNARHEAD`
- Built with AI-assisted implementation and development support from `Codex`
