# AGENTS.md

## Purpose
Operational guide for human and AI contributors working on this repo.
Use this file for execution standards and `MEMORY.md` for historical continuity.

## Project Identity
- Working title: `Pong: Terminal Velocity`
- Vision source: `VISION.md`
- Core focus: deterministic simulation, meaningful spin physics, and symbolic forward-simulation AI.

## Project Values
- Quality over quantity.
- Craft over speed: prioritize correctness, coherence, and long-term maintainability over quick output.
- Deliberate iteration: take the time needed to understand systems before changing them.
- Quality is momentum: fewer well-verified changes are better than many rushed changes.
- Pace is a tool, not a goal: move fast when safe, slow down when complexity or risk increases.

## Project Leadership
- Lead developer: `Codex` (AI agent)
- Game designer and product direction authority: `GNARGNARHEAD` (project owner)
- Technical execution authority: `Codex`, aligned to `VISION.md` and user directives
- Working model: Codex proposes and implements; user can override any decision

## Current Technical Direction (2026-05-22)
- Language: `C++20`
- Windowing/Input: `SDL2`
- Rendering: `OpenGL`
- Build system: `CMake`
- Likely dependencies:
  - `SDL2` (desktop app platform and audio backend)
  - `OpenGL`
  - `libpng` (optional story portraits and menu stickers)
  - `toml++`-style authored content handling through local tooling
  - `libserialport` (optional hardware knob input)

## Engineering Rules
- Keep simulation deterministic and decoupled from rendering.
- Fixed-timestep simulation is the source of truth.
- AI must use the same simulation step function as gameplay.
- Prefer clear, small modules over abstract frameworks.
- Avoid premature graphics complexity; mechanics are priority.
- Add tests for math/physics behaviors when touching sim code.

## Local Machine Safety Rules (2026-02-21)
- Treat host stability as a hard constraint during build/test work.
- Default build parallelism is capped at `-j2` (use `cmake --build build -j2`).
- Do not run strict out-of-tree warning sweeps (for example `/tmp/whacker_warnings8`) unless the user explicitly asks for them.
- Prefer targeted tests for touched areas before any full-suite run.
- Ask before running expensive verification sweeps or repeated rebuild loops.

## Near-Term Milestones
1. Bootstrap project layout + CMake + dependency wiring.
2. Implement deterministic sim core (ball, paddles, spin, collisions).
3. Add simple renderer + HUD and configurable inputs.
4. Implement AI defensive intercept and offensive planner.
5. Add AI-vs-AI accelerated training mode + parameter tuning loop.

## Working Loop Per Task
1. Read `VISION.md` section relevant to the task.
2. Implement the smallest correct vertical slice that runs and is understandable.
3. Add/adjust tests for deterministic behavior.
4. Verify with a reproducible command.
5. Append a session entry in `MEMORY.md`.

## Definition of Done (Per Change)
- Builds locally via documented command.
- Behavior matches relevant vision constraints.
- Determinism risks are addressed or called out.
- Any new knobs are documented in config notes.
- `MEMORY.md` updated with date, what changed, and what is next.
- Quality bar is met even if that means a slower implementation pass.

## Notes Hygiene
- Do not edit `NOTES.md` unless the user explicitly asks.
