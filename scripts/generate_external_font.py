#!/usr/bin/env python3
"""Generate Xteink external .bin fonts from a TTF/OTF.

Format:
  - Direct Unicode codepoint indexing (0x0000..0xFFFF)
  - Offset = codepoint * bytesPerChar
  - Each glyph = bytesPerRow * height bytes
  - 1-bit bitmap, MSB first

Example:
  scripts/generate_external_font.py \
    --font lib/EpdFont/builtinFonts/source/Bookerly/Bookerly-Regular.ttf \
    --name Bookerly --size 20 --width 20 --height 24 \
    --output fonts/Bookerly_20_20x24.bin
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

import freetype


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Generate external Xteink .bin font")
    parser.add_argument("--font", required=True, help="Path to TTF/OTF file")
    parser.add_argument("--name", required=True, help="Font name (used in filename)")
    parser.add_argument("--size", type=int, required=True, help="Font size (pt label in filename)")
    parser.add_argument("--width", type=int, required=True, help="Glyph cell width (px)")
    parser.add_argument("--height", type=int, required=True, help="Glyph cell height (px)")
    parser.add_argument(
        "--baseline-offset",
        type=int,
        default=4,
        help="Baseline offset from bottom (px). Default: 4",
    )
    parser.add_argument(
        "--output",
        help="Output .bin path. Defaults to <name>_<size>_<width>x<height>.bin",
    )
    return parser.parse_args()


def render_glyph(
    face: freetype.Face,
    codepoint: int,
    width: int,
    height: int,
    baseline_y: int,
    bytes_per_row: int,
) -> bytearray:
    buffer = bytearray(bytes_per_row * height)

    glyph_index = face.get_char_index(codepoint)
    if glyph_index == 0:
        return buffer

    face.load_glyph(glyph_index, freetype.FT_LOAD_DEFAULT)
    face.glyph.render(freetype.FT_RENDER_MODE_MONO)
    glyph = face.glyph
    bitmap = glyph.bitmap

    if bitmap.width == 0 or bitmap.rows == 0:
        return buffer

    x0 = max(0, glyph.bitmap_left)
    y0 = baseline_y - glyph.bitmap_top

    for row in range(bitmap.rows):
        target_y = y0 + row
        if target_y < 0 or target_y >= height:
            continue

        row_offset = row * bitmap.pitch
        for col in range(bitmap.width):
            target_x = x0 + col
            if target_x < 0 or target_x >= width:
                continue

            byte = bitmap.buffer[row_offset + (col >> 3)]
            if byte & (0x80 >> (col & 7)):
                out_index = target_y * bytes_per_row + (target_x >> 3)
                out_bit = 7 - (target_x & 7)
                buffer[out_index] |= 1 << out_bit

    return buffer


def main() -> int:
    args = parse_args()

    font_path = Path(args.font)
    if not font_path.exists():
        print(f"Font not found: {font_path}", file=sys.stderr)
        return 1

    width = args.width
    height = args.height
    if width <= 0 or height <= 0:
        print("Width/height must be positive", file=sys.stderr)
        return 1

    output_path = Path(
        args.output
        or f"{args.name}_{args.size}_{width}x{height}.bin"
    )
    output_path.parent.mkdir(parents=True, exist_ok=True)

    face = freetype.Face(str(font_path))
    face.select_charmap(freetype.FT_ENCODING_UNICODE)
    face.set_pixel_sizes(0, args.size)

    bytes_per_row = (width + 7) // 8
    bytes_per_char = bytes_per_row * height
    baseline_y = height - args.baseline_offset

    total_glyphs = 0x10000
    print(
        f"Generating {output_path} ({total_glyphs} glyphs, {bytes_per_char} bytes each)"
    )

    with output_path.open("wb") as f:
        for cp in range(total_glyphs):
            buffer = render_glyph(face, cp, width, height, baseline_y, bytes_per_row)
            f.write(buffer)

            if (cp + 1) % 4096 == 0:
                print(f"  {cp + 1}/{total_glyphs}")

    print(f"Done: {output_path} ({output_path.stat().st_size} bytes)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
