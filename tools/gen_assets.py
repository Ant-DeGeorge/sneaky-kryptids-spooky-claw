#!/usr/bin/env python3
"""Asset generator for Sneaky Kryptid's Spooky Claw (GBC, 64 KB banked).
Emits src/assets.h, src/assets1.c (bank 1), src/assets2.c (bank 2)."""
import math, os, random, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from art import *   # noqa: shared pixel art + helpers

OUT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "src")

# ---------------------------------------------------------------- extra font glyphs
FONT["'"] = [".##..", ".##..", "..#..", ".....", ".....", ".....", "....."]
FONT["/"] = ["....#", "...#.", "...#.", "..#..", ".#...", ".#...", "#...."]
FONT_ORDER = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!?.-x'/"

BIG_STAR = parse([
"......33........",
".....3223.......",
".....3223.......",
"....322223......",
"3333322223333...",
"32222222222223..",
".322222222223...",
"..3222222223....",
"...32222223.....",
"...32222223.....",
"..322233222 3...".replace(' ', '3'),
"..3223..3223....",
".333......333...",
"................",
"................",
"................"], 16)

TOMB = parse([
"................",
".....111111.....",
"....11111111....",
"...1111221111...",
"...1112222111...",
"...1111221111...",
"...1111221111...",
"...1111111111...",
"...1112111111...",
"...1111112111...",
"...1111111111...",
"...1111111111...",
"..311111111113..",
".33333333333333.",
"..............3.",
"................"], 16)
# tombstone: colour 1 = stone (light), 2 = cross (grey), 3 = base shadow

BAT_A = parse([
"....1....1......",
"...11....11.....",
"1..111..111..1..",
"11.1131131 1.11.".replace(' ', '1'),
"1111111111111111",
".1111112211111 .".replace(' ', '1'),
"...111....111...",
"....1......1...."], 16)
BAT_B = parse([
"................",
"................",
"....1....1......",
"...1131131 1....".replace(' ', '1'),
"11111111111111..",
"1.1111122111 1.1".replace(' ', '1'),
"1..11......11..1",
"1..............1"], 16)

# Space Redcap: bubble helmet + jetpack (2 frames: hover, thrust)
RC_SPACE_A = [
"....33333333....",
"...34........3..",
"..34..3443....3.",
"..3..311113...3.",
"..3.3111111 ..3.".replace(' ', '3'),
"..33333333333 3.".replace(' ', '2'),
".322222222222 3.".replace(' ', '2'),
"3222432222432223",
"3222332222332223",
"3122332222332213",
".32222343222223.",
"..322222222223..",
"3..3222222223..3",
"13.3333113333.31",
".3..33.33.33..3.",
".....1.11.1.....",
]
RC_SPACE_B = RC_SPACE_A[:13] + [
"13.3333113333.31",
".3..31.31.31..3.",
"....1.1..1.1....",
]
for f in (RC_SPACE_A, RC_SPACE_B): assert len(f) == 16 and all(len(r) == 16 for r in f)

FLAG = parse([
"3.......",
"32222222",
"32222222",
"32222222",
"32222222",
"3.......",
"3.......",
"3.......",
"3.......",
"........",
"........",
"........",
"........",
"........",
"........",
"........"], 8)
FLAG_B = parse([
"3.......",
"32222...",
"3222222.",
"32222222",
"32222222",
"3222222.",
"3.......",
"3.......",
"3.......",
"........",
"........",
"........",
"........",
"........",
"........",
"........"], 8)

SHIP = parse([
"..........3333..................",
".........322223.................",
"........32111123................",
"3333...3211111123...............",
"3222333211111111233333333333....",
"32222221111111111222222222223...",
"32222221133333311222222222222 ..".replace(' ', '3'),
"3222222113223311222222222222223.",
"32222221133333311222222222222 3.".replace(' ', '3'),
"32222221111111111222222222223...",
"3222333211111111233333333333....",
"3333...3211111123...............",
"........32111123................",
".........322223.................",
"..........3333..................",
"................................"], 32)

# ---------------------------------------------------------------- palettes
def hexrgb(h): h = h.lstrip('#'); return tuple(int(h[i:i+2], 16) for i in (0, 2, 4))

WORLDS = [
 dict(name="PUMPKIN PATCH", night='#3A1F5C',
      interior=['#24103C', '#6A3F9C', '#C8B0F0'], bulbs=['#F07818', '#F8D038', '#FFFFFF']),
 dict(name="GRAVEYARD", night='#1A2038',
      interior=['#0A0E1E', '#4A5A88', '#B8C8E8'], bulbs=['#3878F0', '#A0D0FF', '#FFFFFF']),
 dict(name="HAUNTED FOREST", night='#0C1A10',
      interior=['#1E3A24', '#2E7E36', '#A8E890'], bulbs=['#48C848', '#C8FF70', '#FFFFFF']),
 dict(name="OUTER SPACE", night='#06061C',
      interior=['#101048', '#3860B8', '#C8D8FF'], bulbs=['#F03078', '#30E0F0', '#FFFFFF']),
]
PRIZE_COLS = [   # palettes 3..7 colours 1-3 (colour 0 = world night)
    ['#FFFFFF', '#A8B8E8', '#101018'],  # 3 white
    ['#F07818', '#F8D038', '#402010'],  # 4 orange
    ['#E8C898', '#9A5A2A', '#2A1408'],  # 5 brown
    ['#B888F0', '#FFFFFF', '#101018'],  # 6 violet
    ['#78D848', '#F8D038', '#101018'],  # 7 green
]
REDCAP_COLS = ['#D83030', '#F0B890', '#2A0E14']
def world_bg_pals(w):
    n = w['night']
    return [[n] + w['interior'], ['#140818'] + w['bulbs'], [n] + REDCAP_COLS] + [[n] + p for p in PRIZE_COLS]

TEXT_PAL = ['#3A1F5C', '#F07818', '#F8D038', '#FFFFFF']
RAYS_PAL = ['#3A1F5C', '#2C1548', '#4A2878', '#64389A']
STAR_DIM_PAL = ['#3A1F5C', '#2A1A48', '#3A2A58', '#4A3A68']
OBJ_PALS = [
    ['#000000', '#E0E0F0', '#8088A8', '#101018'],  # 0 claw (runtime)
    ['#000000', '#F8D038', '#F07818', '#FFFFFF'],  # 1 fx
    ['#000000', '#D83030', '#F0B890', '#2A0E14'],  # 2 redcap
] + [['#000000'] + p for p in PRIZE_COLS]           # 3..7 prizes (3 = white: poof, eye shine, bats? no)
BAT_PAL = ['#000000', '#B888F0', '#FFFFFF', '#101018']   # bats use palette 6 (violet) - same as bat prize
CLAW_PALS = [
    ['#000000', '#E0E0F0', '#8088A8', '#101018'],
    ['#000000', '#B8F8A0', '#40C040', '#103010'],
    ['#000000', '#FFF0A0', '#E8B020', '#503000'],
    ['#000000', '#FFFFFF', '#F8D850', '#604000'],
]

# ---------------------------------------------------------------- common tiles (shared, VRAM 0x9000-)
FONT_BASE = 1
STAR_T = FONT_BASE + len(FONT_ORDER)          # 45
BIGSTAR_T = STAR_T + 1                        # 46..49
TOMB_T = BIGSTAR_T + 4                        # 50..53
PRIZE_BG_BASE = 56
SCREEN_BASE = PRIZE_BG_BASE + 32              # 88

def meta16_bg(img): return tiles_from(img, 2, 2)                      # TL TR BL BR
def meta16s(img):                                                     # 8x16 sprite order: TL BL TR BR
    t = tiles_from(img, 2, 2); return [t[0], t[2], t[1], t[3]]
def pair8(top, bottom=None): return [top, bottom or blank(8, 8)]

common = [blank(8, 8)] + [font_tile(c) for c in FONT_ORDER] + [STAR_TILE] + meta16_bg(BIG_STAR) + meta16_bg(TOMB)
while len(common) < PRIZE_BG_BASE: common.append(blank(8, 8))
for img in PRIZE_IMGS: common += meta16_bg(img)
assert len(common) == SCREEN_BASE, len(common)

# ---------------------------------------------------------------- world screens
def circle(img, cx, cy, r, c):
    for y in range(len(img)):
        for x in range(len(img[0])):
            if (x + .5 - cx) ** 2 + (y + .5 - cy) ** 2 <= r * r: img[y][x] = c
    return img

def pile_img(w, seed, cols=(1, 2, 1), base=3, rnd_r=(1.6, 2.6), n=70):
    rnd = random.Random(seed); img = blank(w, 8)
    for y in range(8):
        for x in range(w):
            if y >= 3: img[y][x] = base
    for _ in range(n):
        cx = rnd.uniform(0, w); cy = rnd.uniform(2.5, 8); r = rnd.uniform(*rnd_r); c = rnd.choice(cols)
        for y in range(8):
            for x in range(w):
                d = (x+.5-cx)**2 + (y+.5-cy)**2
                if d <= r*r: img[y][x] = c
                elif d <= (r+.8)**2 and img[y][x] == 0: img[y][x] = base
    return img

def rocks_img(w, seed):
    rnd = random.Random(seed); img = blank(w, 8)
    for _ in range(26):
        cx = rnd.uniform(0, w); cy = rnd.uniform(3, 8); r = rnd.uniform(2, 3.4)
        for y in range(8):
            for x in range(w):
                d = (x+.5-cx)**2 + (y+.5-cy)**2
                if d <= r*r: img[y][x] = 2 if (y < cy) else 1
                elif d <= (r+.7)**2 and img[y][x] == 0: img[y][x] = 1
    return img

def frame_screen(ts, world, pile, pile_pal=0):
    """Cabinet scaffold shared by all worlds. Returns Screen with rows 4-13 interior empty."""
    scr = Screen(ts)
    for y in range(18):
        for x in range(20): scr.put_raw(x, y, 0, 0)
    for y in range(18):
        for x in (0, 19): scr.put(x, y, BULB_A if (y % 2 == 0) else BULB_B, 1)
    for x in range(1, 19):
        scr.put(x, 2, BULB_B if x % 2 else BULB_A, 1)
        scr.put(x, 17, BULB_A if x % 2 else BULB_B, 1)
    # score bar
    for i in range(5):
        for j, t in enumerate(tiles_from(SOCKET, 2, 2)):
            idx, fl = ts.add(t, False)
            scr.put_raw(1 + i*2 + (j & 1), j >> 1, idx, 0)
    pips = []
    for j, t in enumerate(tiles_from(CAP_PIP, 1, 2)):
        idx, fl = ts.add(t, False); pips.append(idx)
    for i in range(5):
        scr.put_raw(12 + i, 0, pips[0], 0); scr.put_raw(12 + i, 1, pips[1], 0)
    for j, t in enumerate(tiles_from(REDCAP_BG, 2, 2)):
        idx, fl = ts.add(t, False)
        scr.put_raw(17 + (j & 1), j >> 1, idx, 2)
    for x in range(1, 19): scr.put(x, 3, RAIL, 0)
    scr.put_img(1, 12, chute_img(), 3, 5, 4)
    for i, t in enumerate(tiles_from(pile, 15, 1)):
        scr.put(4 + i, 16, t, pile_pal)
    scr.put_raw(15, 17, STAR_T, 1)
    scr.put_raw(16, 17, FONT_BASE + FONT_ORDER.index('x'), 1)
    scr.socket_t = scr.map[0][1]; scr.pip_t = pips[0]
    return scr

def stars_on(scr, coords):
    for (x, y, s) in coords: scr.put(x, y, s, 0)

# --- world 1: pumpkin patch
def world1():
    ts = TileSet(SCREEN_BASE); scr = frame_screen(ts, WORLDS[0], pile_img(120, 7), 4)
    stars_on(scr, ((4, 5, STAR1), (8, 4, STAR2), (11, 6, STAR3), (6, 8, STAR3), (16, 9, STAR2), (2, 7, STAR2),
                   (9, 9, STAR1), (17, 7, STAR3), (3, 10, STAR3), (12, 10, STAR2), (14, 4, STAR3), (10, 7, STAR2)))
    scr.put_img(12, 5, MOON, 3, 3, 0)
    scr.put_img(17, 4, web_img(), 2, 2, 0)
    # a couple of pumpkins in the field (interior colours: dark, mid, light)
    pk = blank(16, 8); circle(pk, 5, 5, 3.4, 2); circle(pk, 11, 5, 3.4, 2); pk[1][5] = 1; pk[1][11] = 1
    for x in range(16):
        for y in range(8):
            if pk[y][x] == 2 and (x in (3, 7, 9, 13)) and y > 2: pk[y][x] = 1
    scr.put_img(6, 13, pk, 2, 1, 0); scr.put_img(13, 13, pk, 2, 1, 0)
    return ts, scr

# --- world 2: graveyard
def tomb_bg(h, w=16, cross=True):
    img = blank(w, h)
    for y in range(h):
        for x in range(w):
            dx = abs(x + .5 - w / 2)
            top = 2 + (dx * dx) / (w * 0.9)
            if y >= top and dx < w / 2 - 1.5: img[y][x] = 2
    if cross:
        for y in range(4, 9): img[y][w // 2] = 1; img[y][w // 2 - 1] = 1
        for x in range(w // 2 - 3, w // 2 + 3): img[5][x] = 1; img[6][x] = 1
    for y in range(h): img[y][0] = 0
    return img

def world2():
    ts = TileSet(SCREEN_BASE); scr = frame_screen(ts, WORLDS[1], pile_img(120, 11, cols=(2, 3, 2), base=1, rnd_r=(1.4, 2.2)))
    stars_on(scr, ((5, 4, STAR3), (9, 5, STAR2), (15, 4, STAR3), (3, 8, STAR2), (17, 10, STAR3), (8, 9, STAR3)))
    # cloudy moon
    scr.put_img(12, 5, MOON, 3, 3, 0)
    cloud = blank(32, 8)
    for (cx, cy, r) in ((6, 5, 3.6), (12, 4, 4.6), (19, 4.5, 4.8), (26, 5, 3.8)): circle(cloud, cx, cy, r, 1)
    scr.put_img(11, 7, cloud, 4, 1, 0)
    # dead tree, left
    tree = blank(24, 48)
    for y in range(48):
        w = 2 + (y * 3) // 48
        for x in range(10 - w // 2, 10 + w // 2 + 1): tree[y][x] = 1
    for (y0, dx, L) in ((6, -1, 8), (12, 1, 9), (20, -1, 6), (3, 1, 5)):
        x = 10
        for i in range(L):
            yy = y0 - i // 2; x += dx
            if 0 <= yy < 48 and 0 <= x < 24: tree[yy][x] = 1; tree[yy + 1][x] = 1
    scr.put_img(2, 7, tree, 3, 6, 0)
    # gravestones in the back row + fence
    g = tomb_bg(16)
    scr.put_img(8, 12, g, 2, 2, 0); scr.put_img(15, 12, tomb_bg(16, cross=False), 2, 2, 0); scr.put_img(11, 12, g, 2, 2, 0)
    fence = blank(8, 16)
    for y in range(6, 16): fence[y][2] = 1; fence[y][5] = 1
    for x in range(8): fence[9][x] = 1; fence[13][x] = 1
    for x in (5, 6, 7, 13, 14, 17, 18):
        scr.put_img(x, 12, fence, 1, 2, 0) if x in (5, 6, 7, 13, 14, 17, 18) and x not in (8, 9, 11, 12, 15, 16) else None
    return ts, scr

# --- world 3: haunted forest
def world3():
    ts = TileSet(SCREEN_BASE); scr = frame_screen(ts, WORLDS[2], pile_img(120, 23, cols=(2, 3, 3), base=1, rnd_r=(1.8, 2.8), n=50))
    stars_on(scr, ((8, 4, STAR3), (12, 4, STAR2), (10, 6, STAR3)))
    # two big trunks with branches
    def trunk(mirror):
        img = blank(24, 80)
        for y in range(80):
            w = 7 + (y // 20)
            for x in range(0, w): img[y][x] = 1
            if y % 9 == 0: img[y][w - 1] = 2
        for (y0, L) in ((10, 14), (26, 12), (44, 10), (62, 8)):
            for i in range(L):
                yy = y0 - i // 3
                if 0 <= yy < 80 and 7 + i < 24: img[yy][7 + i] = 1; img[yy + 1][7 + i] = 1
        # owl eyes in a hollow
        circle(img, 3, 36, 2.8, 0); circle(img, 3, 36, 1.2, 3)
        if mirror: img = [row[::-1] for row in img]
        return img
    scr.put_img(1, 4, trunk(False), 3, 10, 0)
    scr.put_img(16, 4, trunk(True), 3, 10, 0)
    # mushrooms + bushes near the floor
    bush = blank(32, 16)
    for (cx, cy, r) in ((7, 11, 5), (16, 9, 6.5), (25, 11, 5)): circle(bush, cx, cy, r, 2)
    for (cx, cy, r) in ((7, 10, 3), (16, 8, 4), (25, 10, 3)): circle(bush, cx, cy, r, 3)
    for y in range(16):
        for x in range(32):
            if bush[y][x] == 3 and (x + y) % 3: bush[y][x] = 2
    scr.put_img(6, 12, bush, 4, 2, 0); scr.put_img(11, 12, bush, 4, 2, 0)
    return ts, scr

# --- world 4: outer space
def world4():
    ts = TileSet(SCREEN_BASE); scr = frame_screen(ts, WORLDS[3], rocks_img(120, 31))
    stars_on(scr, ((3, 4, STAR1), (6, 6, STAR3), (9, 4, STAR2), (14, 5, STAR3), (2, 9, STAR2), (7, 10, STAR3),
                   (11, 8, STAR1), (16, 11, STAR3), (4, 12, STAR3), (12, 12, STAR2), (9, 13, STAR3), (17, 8, STAR2)))
    # ringed planet, right
    planet = blank(40, 32)
    circle(planet, 20, 16, 10, 2)
    for y in range(32):
        for x in range(40):
            if planet[y][x] == 2 and ((x * 3 + y * 5) % 11 < 3): planet[y][x] = 1
    for x in range(40):
        yy = int(16 + (x - 20) * 0.28)
        if 0 <= yy < 32 and ((x - 20) ** 2 / 100 + (yy - 16) ** 2 / 9) > 0.9: planet[yy][x] = 3
    scr.put_img(13, 4, planet, 5, 4, 0)
    # the Sneaky Kryptid ship, top left (interior colours)
    ship = [[(1 if v == 3 else (3 if v == 2 else (2 if v == 1 else 0))) for v in row] for row in SHIP]
    scr.put_img(2, 6, ship, 4, 2, 0)
    return ts, scr

worlds_out = []
for fn in (world1, world2, world3, world4):
    ts, scr = fn()
    assert SCREEN_BASE + len(ts.tiles) <= 256, (fn.__name__, len(ts.tiles))
    worlds_out.append((ts, scr))
    print(fn.__name__, len(ts.tiles), "tiles")
SOCKET_T = worlds_out[0][1].socket_t; PIP_T = worlds_out[0][1].pip_t
for ts, scr in worlds_out: assert scr.socket_t == SOCKET_T and scr.pip_t == PIP_T

# --- title
ts_title = TileSet(SCREEN_BASE); title = Screen(ts_title)
stars_on(title, ((1, 1, STAR1), (5, 2, STAR2), (9, 1, STAR3), (13, 1, STAR1), (2, 5, STAR3),
                 (18, 6, STAR1), (1, 9, STAR2), (18, 9, STAR3), (1, 12, STAR1), (18, 12, STAR3), (10, 14, STAR2), (7, 4, STAR3), (16, 8, STAR2)))
title.put_img(17, 2, MOON, 3, 3, 0)
for i, ch in enumerate("SPOOKY"): title.put_img(4 + i*2, 2, big_letter(ch), 2, 2, 1)
for i, ch in enumerate("CLAW"): title.put_img(6 + i*2, 5, big_letter(ch), 2, 2, 1)
for i, t in enumerate(tiles_from(pile_img(160, 11), 20, 1)): title.put(i, 17, t, 4)
assert SCREEN_BASE + len(ts_title.tiles) <= 256

# --- rays (reveal/stars/card/ending)
ts_rev = TileSet(SCREEN_BASE); rev = Screen(ts_rev)
for y in range(18):
    for x in range(20): rev.put_raw(x, y, 0, 1)
ri = blank(160, 96); cx, cy = 80, 48
for y in range(96):
    for x in range(160):
        a = math.atan2(y + .5 - cy, x + .5 - cx)
        w = int(math.floor((a + math.pi) / (2 * math.pi) * 12 + 0.5)) % 12
        ri[y][x] = 1 + (w % 3)
for i, t in enumerate(tiles_from(ri, 20, 12)): rev.put(i % 20, 3 + i // 20, t, 0, flip=False)

# ---------------------------------------------------------------- sprite tiles (8x16 mode)
SPR = {}; spr = []
def spr_add(name, tiles): SPR[name] = len(spr); spr.extend(tiles)
spr_add("CLAW_OPEN", meta16s(CLAW_OPEN))
spr_add("CLAW_CLOSED", meta16s(CLAW_CLOSED))
carr = CARRIAGE + [[0]*16 for _ in range(8)]
spr_add("CARRIAGE", meta16s(carr))
spr_add("CABLE", pair8(CABLE, CABLE))
spr_add("ARROW_L", pair8(ARROW_L)); spr_add("ARROW_R", pair8(ARROW_R))
spr_add("SPARK_A", pair8(SPARK_A)); spr_add("SPARK_B", pair8(SPARK_B))
spr_add("POOF_A", meta16s(POOF_A)); spr_add("POOF_B", meta16s(POOF_B))
spr_add("BUBBLE", pair8(BUBBLE))
assert len(spr) <= 40, len(spr)
SPR["SWAP"] = 40                      # 40..47: per-world swap area (bats / flag)
while len(spr) < 48: spr.append(blank(8, 8))
SPR["PRIZE"] = 48
for img in PRIZE_IMGS: spr += meta16s(img)
SPR["RC"] = 80
for f in RC_FRAMES:
    b, o = rc_split(f); spr += meta16s(b) + meta16s(o)
assert len(spr) == 128, len(spr)

def rc_pack(f):
    b, o = rc_split(f); return meta16s(b) + meta16s(o)
space_tiles = rc_pack(RC_SPACE_A) + rc_pack(RC_SPACE_B)          # 16 tiles
bat_tiles = []
for img in (BAT_A, BAT_B):
    t = tiles_from(img, 2, 1); bat_tiles += [t[0], blank(8, 8), t[1], blank(8, 8)]   # 8 tiles: 2 frames x (L pair, R pair)
flag_tiles = [FLAG[:8], FLAG[8:], FLAG_B[:8], FLAG_B[8:]]       # 4 tiles: 2 frames
ship_cols = tiles_from(SHIP, 4, 2)
ship_tiles = [ship_cols[0], ship_cols[4], ship_cols[1], ship_cols[5], ship_cols[2], ship_cols[6], ship_cols[3], ship_cols[7]]  # 8 tiles, 4 pairs
# 2x redcap giggle for the lose screen (32 tiles = 8 pairs + 8 overlay pairs)
_b, _o = rc_split(RC_GIGGLE)
def big_pairs(img):
    t = tiles_from(scale2(img), 4, 4); out = []
    for col in range(4):
        for rp in range(2): out += [t[(rp*2)*4 + col], t[(rp*2+1)*4 + col]]
    return out
big_rc = big_pairs(_b) + big_pairs(_o)
big_over_mask = 0
for i in range(8):
    if any(v for t in big_rc[16 + i*2: 18 + i*2] for row in t for v in row): big_over_mask |= 1 << i

# ---------------------------------------------------------------- emit
def c_bytes(name, tiles):
    data = []
    for t in tiles: data += encode(t)
    lines = [f"const uint8_t {name}[{len(data)}] = {{"]
    for i in range(0, len(data), 16): lines.append("  " + ",".join(f"0x{b:02X}" for b in data[i:i+16]) + ",")
    lines.append("};"); return "\n".join(lines)
def c_u8(name, arr):
    flat = [v for row in arr for v in row]
    lines = [f"const uint8_t {name}[{len(flat)}] = {{"]
    for i in range(0, len(flat), 20): lines.append("  " + ",".join(str(v) for v in flat[i:i+20]) + ",")
    lines.append("};"); return "\n".join(lines)
def c_pal(name, pals):
    vals = []
    for p in pals:
        for h in p:
            r, g, b = hexrgb(h); vals.append(f"RGB8({r},{g},{b})")
    return f"const palette_color_t {name}[{len(vals)}] = {{\n  " + ",\n  ".join(vals) + "\n};"

b1 = ["// AUTO-GENERATED by tools/gen_assets.py - do not edit", "#pragma bank 3", '#include "assets.h"', ""]
b1.append(c_bytes("common_tiles", common))
b1.append(c_bytes("sprite_tiles", spr))
b1.append(c_bytes("title_tiles", ts_title.tiles)); b1.append(c_u8("title_map", title.map)); b1.append(c_u8("title_attr", title.attr))
b1.append(c_bytes("reveal_tiles", ts_rev.tiles)); b1.append(c_u8("reveal_map", rev.map)); b1.append(c_u8("reveal_attr", rev.attr))
ts, scr = worlds_out[0]
b1.append(c_bytes("w1_tiles", ts.tiles)); b1.append(c_u8("w1_map", scr.map)); b1.append(c_u8("w1_attr", scr.attr))
b1.append(c_bytes("big_rc_tiles", big_rc))

b2 = ["// AUTO-GENERATED by tools/gen_assets.py - do not edit", "#pragma bank 2", '#include "assets.h"', ""]
for i in (1, 2, 3):
    ts, scr = worlds_out[i]
    b2.append(c_bytes(f"w{i+1}_tiles", ts.tiles)); b2.append(c_u8(f"w{i+1}_map", scr.map)); b2.append(c_u8(f"w{i+1}_attr", scr.attr))
b2.append(c_bytes("space_rc_tiles", space_tiles))
b2.append(c_bytes("bat_tiles", bat_tiles))
b2.append(c_bytes("flag_tiles", flag_tiles))
b2.append(c_bytes("ship_tiles", ship_tiles))

# palettes + small tables live in ROM0 (assets0.c) so they are always reachable
b0 = ["// AUTO-GENERATED by tools/gen_assets.py - do not edit", '#include "assets.h"', ""]
for i, w in enumerate(WORLDS): b0.append(c_pal(f"pal_bg_w{i+1}", world_bg_pals(w)))
b0.append("const palette_color_t * const pal_bg_world[4] = {pal_bg_w1, pal_bg_w2, pal_bg_w3, pal_bg_w4};")
b0.append(c_pal("pal_text", [TEXT_PAL])); b0.append(c_pal("pal_rays", [RAYS_PAL])); b0.append(c_pal("pal_star_dim", [STAR_DIM_PAL]))
b0.append(c_pal("pal_obj", OBJ_PALS)); b0.append(c_pal("pal_claw", CLAW_PALS))
b0.append("const uint8_t prize_pal[8] = {" + ",".join(map(str, PRIZE_PAL)) + "};")
b0.append("const char * const prize_names[8] = {" + ",".join(f'"{n}"' for n in PRIZE_NAMES) + "};")
b0.append("const char * const world_names[4] = {" + ",".join(f'"{w["name"]}"' for w in WORLDS) + "};")
b0.append(f'const char font_order[] = "{FONT_ORDER}";')
b0.append("const uint8_t * const world_tiles[4] = {w1_tiles, w2_tiles, w3_tiles, w4_tiles};")
b0.append("const uint8_t * const world_map[4] = {w1_map, w2_map, w3_map, w4_map};")
b0.append("const uint8_t * const world_attr[4] = {w1_attr, w2_attr, w3_attr, w4_attr};")
b0.append("const uint8_t world_ntiles[4] = {" + ",".join(str(len(ts.tiles)) for ts, _ in worlds_out) + "};")
b0.append("const uint8_t world_bank[4] = {3, 2, 2, 2};")

h = ["// AUTO-GENERATED by tools/gen_assets.py - do not edit", "#ifndef ASSETS_H", "#define ASSETS_H",
     "#include <gb/gb.h>", "#include <gb/cgb.h>", "#include <stdint.h>", "",
     f"#define FONT_BASE {FONT_BASE}", f"#define STAR_T {STAR_T}", f"#define BIGSTAR_T {BIGSTAR_T}", f"#define TOMB_T {TOMB_T}",
     f"#define PRIZE_BG_BASE {PRIZE_BG_BASE}", f"#define SCREEN_BASE {SCREEN_BASE}", f"#define N_COMMON_TILES {len(common)}",
     f"#define N_TITLE_TILES {len(ts_title.tiles)}", f"#define N_REVEAL_TILES {len(ts_rev.tiles)}", f"#define N_SPRITE_TILES {len(spr)}",
     f"#define SOCKET_T {SOCKET_T}", f"#define PIP_T {PIP_T}", f"#define BIG_RC_OVER_MASK 0x{big_over_mask:02X}u",
     "#define RC_IDLE 0", "#define RC_WALK 1", "#define RC_GIGGLE 2", "#define RC_SNEAK 3", "#define RC_BONK 4", "#define RC_POUT 5", ""]
for k, v in SPR.items(): h.append(f"#define SPR_{k} {v}")
h += ["", "extern const uint8_t common_tiles[], sprite_tiles[], title_tiles[], title_map[], title_attr[], reveal_tiles[], reveal_map[], reveal_attr[];",
      "extern const uint8_t w1_tiles[], w1_map[], w1_attr[], w2_tiles[], w2_map[], w2_attr[], w3_tiles[], w3_map[], w3_attr[], w4_tiles[], w4_map[], w4_attr[];",
      "extern const uint8_t big_rc_tiles[], space_rc_tiles[], bat_tiles[], flag_tiles[], ship_tiles[];",
      "extern const palette_color_t * const pal_bg_world[4];",
      "extern const palette_color_t pal_text[], pal_rays[], pal_star_dim[], pal_obj[], pal_claw[];",
      "extern const uint8_t prize_pal[8]; extern const char * const prize_names[8]; extern const char * const world_names[4]; extern const char font_order[];",
      "extern const uint8_t * const world_tiles[4]; extern const uint8_t * const world_map[4]; extern const uint8_t * const world_attr[4];",
      "extern const uint8_t world_ntiles[4]; extern const uint8_t world_bank[4];", "", "#endif"]

os.makedirs(OUT_DIR, exist_ok=True)
open(os.path.join(OUT_DIR, "assets0.c"), "w").write("\n".join(b0) + "\n")
open(os.path.join(OUT_DIR, "assets1.c"), "w").write("\n".join(b1) + "\n")
open(os.path.join(OUT_DIR, "assets2.c"), "w").write("\n".join(b2) + "\n")
open(os.path.join(OUT_DIR, "assets.h"), "w").write("\n".join(h) + "\n")

if "--preview" in sys.argv:
    from PIL import Image
    S = 3
    def render(scr, ts, pals):
        im = Image.new("RGB", (160 * S, 144 * S))
        allt = common + ts.tiles
        for ty in range(18):
            for tx in range(20):
                idx = scr.map[ty][tx]; at = scr.attr[ty][tx]; pal = [hexrgb(c) for c in pals[at & 7]]
                t = allt[idx]
                if at & 0x20: t = hflip_t(t)
                if at & 0x40: t = vflip_t(t)
                for y in range(8):
                    for x in range(8):
                        for dy in range(S):
                            for dx in range(S): im.putpixel(((tx*8+x)*S+dx, (ty*8+y)*S+dy), pal[t[y][x]])
        return im
    sheet = Image.new("RGB", (4 * 160 * S + 30, 144 * S), "white")
    for i, (ts, scr) in enumerate(worlds_out):
        sheet.paste(render(scr, ts, world_bg_pals(WORLDS[i])), (i * (160 * S + 10), 0))
    sheet.save(os.path.join(OUT_DIR, "..", "build", "worlds_preview.png"))
    pals = world_bg_pals(WORLDS[0]); pals[1] = TEXT_PAL
    render(title, ts_title, pals).save(os.path.join(OUT_DIR, "..", "build", "title_preview.png"))
print("assets written")
