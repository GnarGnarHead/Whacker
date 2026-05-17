# Portrait Asset Pipeline (Lossless Optimize + Cleanup)

This pipeline preserves full portrait quality in runtime while reducing PNG file size with lossless compression.

## Goals

- Runtime portraits should preserve full source detail (no forced downsample or palette quantization).
- Portrait files should stay lightweight.
- New assets should follow the same repeatable process.
- No blind global tuning.

## Source Of Truth

- Profile: `portrait_normalization_profile.json`
- Processor: `normalize_portraits.sh`
- Canonical runtime files: `*_portrait.png` in this folder.

## Strict Profile Contract

The profile schema is strict and validated before processing.

- Top-level keys must be exactly: `version`, `default`, `overrides`.
- `default` may only contain:
  - `fuzz_percent` (number)
- Each `overrides` entry key must be a canonical portrait filename (`*_portrait.png`).
- Each override object may only contain:
  - `fuzz_percent` (optional number)

Any unknown or unsupported key causes script failure.

## What The Processor Does

For each portrait listed in the profile:

1. Removes corner-connected background via flood fill (`fuzz_percent` per asset).
2. Writes a lossless optimized PNG (`-strip` + compression settings).
3. Preserves source dimensions and color fidelity (no resize, no `PNG8`, no palette reduction).

## Run It

From repo root:

```bash
story/characters/art/normalize_portraits.sh
```

Schema-only validation (no file writes):

```bash
story/characters/art/normalize_portraits.sh --validate-only
```

## Validate Output

### Dimensions + alpha

```bash
identify -format '%f %wx%h opaque:%[opaque]\n' story/characters/art/*portrait.png
```

Expected:

- source dimensions are preserved (no forced `64x64` conversion)
- `opaque:False` for portraits that include transparency

### File size audit

```bash
ls -lh story/characters/art/*portrait.png
```

Expected:

- files reduced versus unoptimized exports, with no visible quality loss

### Optional subject-bounds sanity check

```bash
identify -format '%f trim:%@\n' story/characters/art/*portrait.png
```

Use this to inspect visible subject bounds if a portrait reads too small/large in-game.

## Per-Asset Tuning Rules

All per-asset tuning is in `portrait_normalization_profile.json` under `overrides`.

- `fuzz_percent`
  - Increase if background haze remains.
  - Decrease if edges (hair/jacket) get cut.

## Adding A New Portrait

1. Add new `*_portrait.png` file to this folder.
2. Add an override entry in `portrait_normalization_profile.json`.
3. Run the normalization script.
4. Validate dimensions, alpha, and file size.
5. Preview in-game; if cleanup removed desired edge detail, lower `fuzz_percent`.

## Common Failure Modes

- **Background halo visible**
  - Increase `fuzz_percent` slightly.
- **Looks blurry**
  - Re-export cleaner source art; this pipeline is lossless and will not add blur by itself.
