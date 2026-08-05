#!/usr/bin/env python3
"""Regenerate jni/game/gfx/speed_menu.c -- the game-speed button (1x/2x/4x).

The button must look exactly like the original Build/Cancel buttons, so the
frame artwork and palette are taken straight from Noda's build_menu.c: this
script copies a build_menu frame, erases the baked-in "Build" lettering by
extending the button's own fill colour across the text band, and stamps
"1x"/"2x"/"4x" using serif glyphs drawn to match the original letterforms
(1px stems, white fill idx 68, dark outline idx 54).

Output frames: 0=1x 1=2x 2=4x (normal), 3=1x 4=2x 5=4x (pressed).

Usage:  python3 tools/gen_speed_menu.py
"""
import os
import re
from collections import Counter

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC = os.path.join(ROOT, 'jni/game/gfx/build_menu.c')
DST = os.path.join(ROOT, 'jni/game/gfx/speed_menu.c')

W, H = 64, 32
WHITE, DARK = 68, 54          # text fill / outline, as used by "Build"
TEXT_X0, TEXT_X1 = 12, 54     # columns covered by the original lettering
TEXT_Y0, TEXT_Y1 = 4, 14      # rows covered by the original lettering
CLEAN_X0, CLEAN_X1 = 45, 58   # flat area sampled for the replacement fill

# Serif glyphs matching the original font: 1px stems, 8 rows tall.
GLYPHS = {
    '1': [" ## ", "### ", " ## ", " ## ", " ## ", " ## ", " ## ", "####"],
    '2': [" ### ", "#   #", "    #", "   # ", "  #  ", " #   ", "#    ", "#####"],
    '4': ["   # ", "  ## ", " # # ", "#  # ", "#####", "   # ", "   # ", "  ###"],
    'x': ["      ", "      ", "      ", "##  ##", " #  # ", "  ##  ", " #  # ", "##  ##"],
}
LABELS = ('1x', '2x', '4x')


def load_frames():
    body = open(SRC).read().split('{', 1)[1].split('}')[0]
    return [int(x) for x in re.findall(r'\d+', body)]


def frame_to_grid(data, f):
    """Un-swizzle one 64x32 frame from DS 8x8-tile order into a pixel grid."""
    g = [[0] * W for _ in range(H)]
    base = f * (W * H)
    for ty in range(H // 8):
        for tx in range(W // 8):
            t = base + (ty * (W // 8) + tx) * 64
            for y in range(8):
                for x in range(8):
                    g[ty * 8 + y][tx * 8 + x] = data[t + y * 8 + x]
    return g


def grid_to_frame(g):
    """Re-swizzle a pixel grid back into DS 8x8-tile order."""
    out = []
    for ty in range(H // 8):
        for tx in range(W // 8):
            for y in range(8):
                for x in range(8):
                    out.append(g[ty * 8 + y][tx * 8 + x])
    return out


def make(data, label, src_frame):
    g = [row[:] for row in frame_to_grid(data, src_frame)]

    # erase "Build": refill the text band with the row's own dominant colour
    for y in range(TEXT_Y0, TEXT_Y1):
        fill = Counter(g[y][x] for x in range(CLEAN_X0, CLEAN_X1)).most_common(1)[0][0]
        for x in range(TEXT_X0, TEXT_X1):
            g[y][x] = fill

    # stamp the label, centred, outlined like the original
    glyphs = [GLYPHS[c] for c in label]
    total = sum(len(gl[0]) for gl in glyphs) + (len(label) - 1)
    x0 = (W - total) // 2
    px = set()
    for gl in glyphs:
        for y, row in enumerate(gl):
            for x, ch in enumerate(row):
                if ch == '#':
                    px.add((x0 + x, 5 + y))
        x0 += len(gl[0]) + 1
    for (x, y) in px:
        for dy in (-1, 0, 1):
            for dx in (-1, 0, 1):
                if (x + dx, y + dy) not in px and 0 <= y + dy < H and 0 <= x + dx < W:
                    g[y + dy][x + dx] = DARK
    for (x, y) in px:
        g[y][x] = WHITE
    return grid_to_frame(g)


def main():
    data = load_frames()
    out = []
    for src_frame in (0, 2):          # build_menu: 0 = normal, 2 = pressed
        for label in LABELS:
            out += make(data, label, src_frame)

    lines = [', '.join(str(v) for v in out[i:i + 16]) + ', '
             for i in range(0, len(out), 16)]
    open(DST, 'w').write(
        "//Sprite created for the Android port: game-speed button (1x/2x/4x)\n"
        "//Frame layout: 0=1x 1=2x 2=4x (normal), 3=1x 4=2x 5=4x (pressed)\n"
        "//Derived from build_menu.c (Noda) -- same frame artwork and palette\n"
        "//(build_menu_Pal); only the label glyphs differ.\n"
        "//Regenerate with tools/gen_speed_menu.py\n\n"
        "const unsigned char speed_menu_Sprite[%d] __attribute__ ((aligned (4))) = {\n%s};\n"
        % (len(out), '\n'.join(lines)))
    print('wrote %s (%d bytes)' % (DST, len(out)))


if __name__ == '__main__':
    main()
