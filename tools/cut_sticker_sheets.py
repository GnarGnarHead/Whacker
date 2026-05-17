#!/usr/bin/env python3

from __future__ import annotations

import argparse
import json
import math
import re
from dataclasses import dataclass
from pathlib import Path

from PIL import Image, ImageChops, ImageFilter


@dataclass(frozen=True)
class Box:
    x0: int
    y0: int
    x1: int
    y1: int

    def width(self) -> int:
        return max(0, self.x1 - self.x0)

    def height(self) -> int:
        return max(0, self.y1 - self.y0)

    def area(self) -> int:
        return self.width() * self.height()

    def expanded(self, pad: int, max_w: int, max_h: int) -> "Box":
        return Box(
            x0=max(0, self.x0 - pad),
            y0=max(0, self.y0 - pad),
            x1=min(max_w, self.x1 + pad),
            y1=min(max_h, self.y1 + pad),
        )

    def intersects(self, other: "Box") -> bool:
        return not (self.x1 <= other.x0 or other.x1 <= self.x0 or self.y1 <= other.y0 or other.y1 <= self.y0)

    def union(self, other: "Box") -> "Box":
        return Box(
            x0=min(self.x0, other.x0),
            y0=min(self.y0, other.y0),
            x1=max(self.x1, other.x1),
            y1=max(self.y1, other.y1),
        )


@dataclass(frozen=True)
class AlphaComponent:
    box: Box
    count: int
    touches_border: bool


def sanitize_stem(path: Path) -> str:
    stem = path.stem.lower()
    stem = re.sub(r"[^a-z0-9]+", "_", stem).strip("_")
    return stem or "sheet"


def otsu_threshold(hist: list[int]) -> int:
    total = sum(hist)
    if total <= 0:
        return 128

    sum_total = 0
    for i, count in enumerate(hist):
        sum_total += i * count

    sum_bg = 0
    weight_bg = 0
    max_between = -1.0
    threshold = 128

    for t in range(256):
        weight_bg += hist[t]
        if weight_bg == 0:
            continue
        weight_fg = total - weight_bg
        if weight_fg == 0:
            break

        sum_bg += t * hist[t]
        mean_bg = sum_bg / weight_bg
        mean_fg = (sum_total - sum_bg) / weight_fg
        between = float(weight_bg) * float(weight_fg) * (mean_bg - mean_fg) ** 2
        if between > max_between:
            max_between = between
            threshold = t

    return int(threshold)


def binarize_by_diff(image_rgb: Image.Image, blur_radius: float, threshold_floor: int) -> Image.Image:
    blur = image_rgb.filter(ImageFilter.GaussianBlur(radius=blur_radius))
    diff = ImageChops.difference(image_rgb, blur).convert("L")
    t = otsu_threshold(diff.histogram())
    t = max(threshold_floor, t)
    return diff.point(lambda p: 255 if p > t else 0)


def close_mask(mask: Image.Image, dilate_size: int, erode_size: int) -> Image.Image:
    if dilate_size >= 3:
        mask = mask.filter(ImageFilter.MaxFilter(dilate_size))
    if erode_size >= 3:
        mask = mask.filter(ImageFilter.MinFilter(erode_size))
    return mask


def find_components(mask: Image.Image) -> list[tuple[Box, int]]:
    w, h = mask.size
    pixels = mask.load()
    visited = bytearray(w * h)
    components: list[tuple[Box, int]] = []

    for y in range(h):
        row = y * w
        for x in range(w):
            idx = row + x
            if visited[idx] != 0:
                continue
            if pixels[x, y] == 0:
                continue
            visited[idx] = 1
            stack: list[tuple[int, int]] = [(x, y)]
            min_x = max_x = x
            min_y = max_y = y
            count = 0
            while stack:
                cx, cy = stack.pop()
                count += 1
                if cx < min_x:
                    min_x = cx
                if cx > max_x:
                    max_x = cx
                if cy < min_y:
                    min_y = cy
                if cy > max_y:
                    max_y = cy

                nx = cx - 1
                ny = cy
                if 0 <= nx < w and pixels[nx, ny] != 0:
                    nidx = ny * w + nx
                    if visited[nidx] == 0:
                        visited[nidx] = 1
                        stack.append((nx, ny))
                nx = cx + 1
                if 0 <= nx < w and pixels[nx, ny] != 0:
                    nidx = ny * w + nx
                    if visited[nidx] == 0:
                        visited[nidx] = 1
                        stack.append((nx, ny))
                nx = cx
                ny = cy - 1
                if 0 <= ny < h and pixels[nx, ny] != 0:
                    nidx = ny * w + nx
                    if visited[nidx] == 0:
                        visited[nidx] = 1
                        stack.append((nx, ny))
                ny = cy + 1
                if 0 <= ny < h and pixels[nx, ny] != 0:
                    nidx = ny * w + nx
                    if visited[nidx] == 0:
                        visited[nidx] = 1
                        stack.append((nx, ny))

            components.append((Box(min_x, min_y, max_x + 1, max_y + 1), count))

    return components


def alpha_components(mask: Image.Image) -> list[tuple[AlphaComponent, list[int]]]:
    w, h = mask.size
    pixels = mask.load()
    visited = bytearray(w * h)
    components: list[tuple[AlphaComponent, list[int]]] = []

    for y in range(h):
        row = y * w
        for x in range(w):
            idx = row + x
            if visited[idx] != 0:
                continue
            if pixels[x, y] == 0:
                continue
            visited[idx] = 1
            stack: list[tuple[int, int]] = [(x, y)]
            comp_pixels: list[int] = []
            min_x = max_x = x
            min_y = max_y = y
            touches_border = x == 0 or y == 0 or x == (w - 1) or y == (h - 1)
            while stack:
                cx, cy = stack.pop()
                cidx = cy * w + cx
                comp_pixels.append(cidx)
                if cx < min_x:
                    min_x = cx
                if cx > max_x:
                    max_x = cx
                if cy < min_y:
                    min_y = cy
                if cy > max_y:
                    max_y = cy
                if cx == 0 or cy == 0 or cx == (w - 1) or cy == (h - 1):
                    touches_border = True

                nx = cx - 1
                ny = cy
                if 0 <= nx < w and pixels[nx, ny] != 0:
                    nidx = ny * w + nx
                    if visited[nidx] == 0:
                        visited[nidx] = 1
                        stack.append((nx, ny))
                nx = cx + 1
                if 0 <= nx < w and pixels[nx, ny] != 0:
                    nidx = ny * w + nx
                    if visited[nidx] == 0:
                        visited[nidx] = 1
                        stack.append((nx, ny))
                nx = cx
                ny = cy - 1
                if 0 <= ny < h and pixels[nx, ny] != 0:
                    nidx = ny * w + nx
                    if visited[nidx] == 0:
                        visited[nidx] = 1
                        stack.append((nx, ny))
                ny = cy + 1
                if 0 <= ny < h and pixels[nx, ny] != 0:
                    nidx = ny * w + nx
                    if visited[nidx] == 0:
                        visited[nidx] = 1
                        stack.append((nx, ny))

            components.append(
                (
                    AlphaComponent(
                        box=Box(min_x, min_y, max_x + 1, max_y + 1),
                        count=len(comp_pixels),
                        touches_border=touches_border,
                    ),
                    comp_pixels,
                )
            )

    return components


def cleanup_alpha_components(
    alpha: Image.Image,
    *,
    mode: str,
    min_area_frac: float,
    min_area_px: int,
) -> tuple[Image.Image, dict[str, int | str]]:
    components = alpha_components(alpha)
    if not components:
        return alpha, {
            "fragment_cleanup_mode": mode,
            "components_before": 0,
            "components_after": 0,
            "removed_pixels": 0,
            "kept_component_pixels": 0,
        }

    before_pixels = sum(comp.count for comp, _ in components)
    if mode == "off":
        return alpha, {
            "fragment_cleanup_mode": mode,
            "components_before": len(components),
            "components_after": len(components),
            "removed_pixels": 0,
            "kept_component_pixels": before_pixels,
        }

    main_idx = max(range(len(components)), key=lambda i: components[i][0].count)
    main_comp = components[main_idx][0]
    main_size = main_comp.count
    main_box = main_comp.box
    main_center_box = main_box.expanded(6, alpha.width, alpha.height)
    min_area_frac = max(0.0, float(min_area_frac))
    min_area_px = max(1, int(min_area_px))
    large_component_threshold = max(min_area_px, int(main_size * min_area_frac))
    conservative_threshold = max(48, min_area_px // 3, int(main_size * min_area_frac * 0.35))
    overlap_ratio_threshold = 0.33

    keep: set[int] = set()
    for idx, (comp, _) in enumerate(components):
        if idx == main_idx:
            keep.add(idx)
            continue
        if mode == "strict":
            continue
        if mode == "balanced":
            if comp.touches_border:
                continue
            center_x = 0.5 * (comp.box.x0 + comp.box.x1)
            center_y = 0.5 * (comp.box.y0 + comp.box.y1)
            center_in_main = (
                main_center_box.x0 <= center_x < main_center_box.x1
                and main_center_box.y0 <= center_y < main_center_box.y1
            )
            overlap_w = max(0, min(comp.box.x1, main_box.x1) - max(comp.box.x0, main_box.x0))
            overlap_h = max(0, min(comp.box.y1, main_box.y1) - max(comp.box.y0, main_box.y0))
            overlap_area = overlap_w * overlap_h
            box_area = comp.box.area()
            overlap_ratio = (overlap_area / float(box_area)) if box_area > 0 else 0.0
            if center_in_main or overlap_ratio >= overlap_ratio_threshold or comp.count >= large_component_threshold:
                keep.add(idx)
            continue
        if not comp.touches_border:
            keep.add(idx)
            continue
        if mode == "conservative":
            if comp.count >= conservative_threshold:
                keep.add(idx)
            continue
        keep.add(idx)

    w, h = alpha.size
    out = bytearray(w * h)
    after_pixels = 0
    for idx, (comp, comp_pixels) in enumerate(components):
        if idx not in keep:
            continue
        after_pixels += comp.count
        for pidx in comp_pixels:
            out[pidx] = 255

    cleaned = Image.frombytes("L", (w, h), bytes(out))
    return cleaned, {
        "fragment_cleanup_mode": mode,
        "components_before": len(components),
        "components_after": len(keep),
        "removed_pixels": max(0, before_pixels - after_pixels),
        "kept_component_pixels": after_pixels,
    }


def merge_overlapping_boxes(boxes: list[Box], pad: int, max_w: int, max_h: int) -> list[Box]:
    if len(boxes) <= 1:
        return boxes

    def intersection_area(a: Box, b: Box) -> int:
        ix0 = max(a.x0, b.x0)
        iy0 = max(a.y0, b.y0)
        ix1 = min(a.x1, b.x1)
        iy1 = min(a.y1, b.y1)
        if ix1 <= ix0 or iy1 <= iy0:
            return 0
        return (ix1 - ix0) * (iy1 - iy0)

    expanded = [b.expanded(pad, max_w, max_h) for b in boxes]
    merge_ratio = 0.60
    adj: list[list[int]] = [[] for _ in boxes]
    for i in range(len(boxes)):
        for j in range(i + 1, len(boxes)):
            inter = intersection_area(expanded[i], expanded[j])
            if inter <= 0:
                continue
            denom = min(boxes[i].area(), boxes[j].area())
            if denom <= 0:
                continue
            if (inter / float(denom)) < merge_ratio:
                continue
            adj[i].append(j)
            adj[j].append(i)

    seen = [False] * len(boxes)
    merged: list[Box] = []
    for i in range(len(boxes)):
        if seen[i]:
            continue
        seen[i] = True
        stack = [i]
        union_box = boxes[i]
        while stack:
            u = stack.pop()
            for v in adj[u]:
                if seen[v]:
                    continue
                seen[v] = True
                stack.append(v)
                union_box = union_box.union(boxes[v])
        merged.append(union_box)

    return merged


def fill_holes(mask: Image.Image) -> Image.Image:
    w, h = mask.size
    inv = ImageChops.invert(mask)
    inv_px = inv.load()
    visited = bytearray(w * h)
    stack: list[tuple[int, int]] = []

    def push_if_white(x: int, y: int) -> None:
        if inv_px[x, y] == 0:
            return
        idx = y * w + x
        if visited[idx] != 0:
            return
        visited[idx] = 1
        stack.append((x, y))

    for x in range(w):
        push_if_white(x, 0)
        push_if_white(x, h - 1)
    for y in range(h):
        push_if_white(0, y)
        push_if_white(w - 1, y)

    while stack:
        cx, cy = stack.pop()
        for nx, ny in ((cx - 1, cy), (cx + 1, cy), (cx, cy - 1), (cx, cy + 1)):
            if not (0 <= nx < w and 0 <= ny < h):
                continue
            if inv_px[nx, ny] == 0:
                continue
            nidx = ny * w + nx
            if visited[nidx] != 0:
                continue
            visited[nidx] = 1
            stack.append((nx, ny))

    holes = Image.new("L", (w, h), 0)
    holes_px = holes.load()
    for y in range(h):
        row = y * w
        for x in range(w):
            if inv_px[x, y] == 0:
                continue
            if visited[row + x] == 0:
                holes_px[x, y] = 255

    return ImageChops.lighter(mask, holes)


def extract_sticker_alpha(crop_rgb: Image.Image) -> Image.Image:
    w, h = crop_rgb.size
    blur_radius = max(6.0, min(w, h) / 18.0)
    mask = binarize_by_diff(crop_rgb, blur_radius=blur_radius, threshold_floor=12)
    mask = close_mask(mask, dilate_size=5, erode_size=3)
    mask = fill_holes(mask)
    mask = close_mask(mask, dilate_size=3, erode_size=3)
    return mask


def detect_sticker_boxes(
    sheet: Image.Image,
    *,
    merge: bool = True,
    min_count: int = 120,
    max_area_frac: float = 0.25,
) -> list[Box]:
    w, h = sheet.size
    scale = max(2, int(math.floor(min(w, h) / 256.0)))
    small_w = max(1, w // scale)
    small_h = max(1, h // scale)
    small = sheet.convert("RGB").resize((small_w, small_h), resample=Image.Resampling.BILINEAR)

    blur_radius = max(4.0, min(small_w, small_h) / 14.0)
    mask = binarize_by_diff(small, blur_radius=blur_radius, threshold_floor=10)

    # Favor many smaller components over a single merged slab: keep morphology gentle.
    mask = close_mask(mask, dilate_size=3, erode_size=3)

    components = find_components(mask)
    boxes: list[Box] = []
    image_area = small_w * small_h

    for box, count in components:
        bw = box.width()
        bh = box.height()
        area = box.area()
        if area <= 0:
            continue
        if area > int(image_area * 0.90):
            continue
        if bw < 6 or bh < 6:
            continue
        if count < max(1, int(min_count)):
            continue
        # Reject big slabs that usually represent the sheet background gradient.
        if max_area_frac > 0.0 and area > int(image_area * float(max_area_frac)):
            continue
        boxes.append(box)

    if merge:
        # Merging is intentionally conservative to avoid cascades where unions create
        # new overlaps and collapse an entire sheet into one giant box.
        boxes = merge_overlapping_boxes(boxes, pad=1, max_w=small_w, max_h=small_h)
    boxes = sorted(boxes, key=lambda b: (b.y0, b.x0))

    scaled: list[Box] = []
    for box in boxes:
        x0 = box.x0 * scale
        y0 = box.y0 * scale
        x1 = min(w, box.x1 * scale)
        y1 = min(h, box.y1 * scale)
        scaled.append(Box(x0, y0, x1, y1))

    return scaled


def main() -> int:
    parser = argparse.ArgumentParser(description="Cut sticker sheets into individual transparent PNG assets.")
    parser.add_argument(
        "sheets",
        nargs="+",
        type=Path,
        help="Sticker sheet image paths, or directories containing images (png/jpg/webp).",
    )
    parser.add_argument("--out", type=Path, default=Path("art_exports/stickers"), help="Output directory.")
    parser.add_argument("--pad", type=int, default=24, help="Extra pixels around detected bounds before masking.")
    parser.add_argument("--min-count", type=int, default=120, help="Minimum connected-component pixel count.")
    parser.add_argument(
        "--max-area-frac",
        type=float,
        default=0.25,
        help="Reject detected regions larger than this fraction of the downscaled image area.",
    )
    parser.add_argument(
        "--fragment-cleanup",
        choices=("off", "balanced", "strict", "conservative"),
        default="balanced",
        help="Post-process alpha to remove border-touching fragment sparkle cutoffs.",
    )
    parser.add_argument(
        "--fragment-min-area-frac",
        type=float,
        default=0.06,
        help="Balanced/Conservative: minimum fragment size as fraction of largest component size.",
    )
    parser.add_argument(
        "--fragment-min-area-px",
        type=int,
        default=180,
        help="Balanced/Conservative: minimum fragment size in pixels.",
    )
    parser.add_argument("--no-merge", action="store_true", help="Do not merge overlapping detected regions.")
    args = parser.parse_args()

    out_root: Path = args.out
    out_root.mkdir(parents=True, exist_ok=True)
    manifest: dict[str, object] = {"version": 1, "sheets": []}

    sheet_paths: list[Path] = []
    for raw in args.sheets:
        if raw.is_dir():
            for child in sorted(raw.iterdir()):
                if not child.is_file():
                    continue
                if child.suffix.lower() not in (".png", ".jpg", ".jpeg", ".webp"):
                    continue
                sheet_paths.append(child)
            continue
        sheet_paths.append(raw)

    for sheet_path in sheet_paths:
        sheet_path = sheet_path.resolve()
        sheet = Image.open(sheet_path)
        sheet.load()
        sheet_rgba = sheet.convert("RGBA")
        w, h = sheet_rgba.size

        sheet_id = sanitize_stem(sheet_path)
        sheet_out = out_root / sheet_id
        sheet_out.mkdir(parents=True, exist_ok=True)
        for old_asset in sheet_out.glob(f"{sheet_id}_*.png"):
            old_asset.unlink()

        boxes = detect_sticker_boxes(
            sheet_rgba,
            merge=not args.no_merge,
            min_count=args.min_count,
            max_area_frac=args.max_area_frac,
        )
        sheet_entries: list[dict[str, object]] = []

        for idx, box in enumerate(boxes, start=1):
            pad = max(8, int(args.pad))
            padded = box.expanded(pad, w, h)
            crop = sheet_rgba.crop((padded.x0, padded.y0, padded.x1, padded.y1))
            crop_rgb = crop.convert("RGB")
            alpha = extract_sticker_alpha(crop_rgb)
            alpha, cleanup_meta = cleanup_alpha_components(
                alpha,
                mode=args.fragment_cleanup,
                min_area_frac=args.fragment_min_area_frac,
                min_area_px=args.fragment_min_area_px,
            )

            crop.putalpha(alpha)
            tight = alpha.getbbox()
            if tight is not None:
                tx0, ty0, tx1, ty1 = tight
                trim_pad = 6
                tx0 = max(0, tx0 - trim_pad)
                ty0 = max(0, ty0 - trim_pad)
                tx1 = min(crop.width, tx1 + trim_pad)
                ty1 = min(crop.height, ty1 + trim_pad)
                crop = crop.crop((tx0, ty0, tx1, ty1))

            asset_name = f"{sheet_id}_{idx:03d}.png"
            asset_path = sheet_out / asset_name
            crop.save(asset_path, format="PNG", optimize=True)

            sheet_entries.append(
                {
                    "id": f"{sheet_id}_{idx:03d}",
                    "source_sheet": str(sheet_path),
                    "source_box": {"x": box.x0, "y": box.y0, "w": box.width(), "h": box.height()},
                    "output": str(asset_path),
                    "components_before": cleanup_meta["components_before"],
                    "components_after": cleanup_meta["components_after"],
                    "removed_pixels": cleanup_meta["removed_pixels"],
                    "kept_component_pixels": cleanup_meta["kept_component_pixels"],
                    "fragment_cleanup_mode": cleanup_meta["fragment_cleanup_mode"],
                }
            )

        manifest["sheets"].append(
            {
                "sheet": str(sheet_path),
                "sheet_id": sheet_id,
                "size": {"w": w, "h": h},
                "count": len(sheet_entries),
                "assets": sheet_entries,
            }
        )

    manifest_path = out_root / "manifest.json"
    manifest_path.write_text(json.dumps(manifest, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    print(f"Wrote {manifest_path}")
    for sheet in manifest["sheets"]:
        print(f"- {sheet['sheet_id']}: {sheet['count']} assets")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
