#!/usr/bin/env python3
"""Per-level balance bot (symbol build). args: rom map level frames seed aim_sigma glow_aware react_min react_max
Plays the given level repeatedly (poking level back after each round) and reports win rate + stars."""
import sys, re, random
from pyboy import PyBoy
rom, mapf = sys.argv[1], sys.argv[2]
LV = int(sys.argv[3]); FR = int(sys.argv[4]); rnd = random.Random(int(sys.argv[5]))
SIG = float(sys.argv[6]); GLOW = float(sys.argv[7]); RMIN, RMAX = int(sys.argv[8]), int(sys.argv[9]); WAITBATS = len(sys.argv) > 10 and sys.argv[10] == '1'
syms = {m.group(2): int(m.group(1), 16) for m in re.finditer(r"([0-9A-F]{8})\s+(_\w+)", open(mapf).read())}
p = PyBoy(rom, window="null", cgb=True, sound_emulated=False); M = p.memory
rd = lambda n, o=0: M[syms[n[:9]] + o]
def wr(n, v, o=0): M[syms[n[:9]] + o] = v
for _ in range(200): p.tick()
wr("_level", LV); p.button("a")
f = 0; held = None; wait = 0; mode = "pick"; target = 0; react = None
res = []; prev = None; last_change = 0; stuck = False
sneaks = shoos = steals = jig = cracks = knocks = 0
while f < FR:
    st, ps = rd("_state"), rd("_pstate")
    if (st, ps) != prev:
        last_change = f
        if st == 4 and (prev is None or prev[0] != 4): res.append((1, rd("_pscore"), rd("_rscore"), rd("_stars_ea")))
        if st == 3 and (prev is None or prev[0] != 3): res.append((0, rd("_pscore"), rd("_rscore"), 0))
        if st == 2 and ps == 12: sneaks += 1
        if st == 2 and ps == 13: shoos += 1
        if st == 2 and ps == 10: steals += 1
        if st == 2 and ps == 5: jig += 1
        prev = (st, ps)
    if st == 2 and rd("_broke_sto") and ps == 3 and rd("_ptimer") == 9: cracks += 1
    if st == 2 and rd("_knocked") and ps == 2 and rd("_shake_t") == 6: knocks += 1
    if f - last_change > 4000 and not (st == 2 and ps in (1, 12)):
        print("STUCK", st, ps, f); stuck = True; break
    if held: p.button_release(held); held = None
    if wait: wait -= 1
    elif st in (0, 1, 3, 4, 5):
        # keep replaying the same level: pin the level before leaving stars/lose screens
        wr("_level", LV)
        if rnd.random() < 0.03: p.button("a"); wait = 20
    elif st == 2 and ps == 12:
        if react is None: react = rnd.randint(RMIN, RMAX)
        react -= 1
        if react <= 0: p.button("a"); react = None; wait = 10
    elif st == 2 and ps == 1:
        react = None
        x = rd("_clawX")
        if mode == "pick":
            occ = [s for s in range(5) if rd("_slot_type", s) != 0xFF]
            if occ:
                s = rnd.choice(occ)
                target = int(round(48 + 24 * s + rnd.gauss(0, SIG)))
                target = max(36, min(144, target)); mode = "move"
        elif mode == "move":
            if abs(x - target) <= 1: mode = "settle"; wait = rnd.randint(5, 25)
            elif x < target: p.button_press("right"); held = "right"
            else: p.button_press("left"); held = "left"
        elif mode == "settle":
            glow = rd("_claw_pal_") in (1, 3)
            bats_near = False
            if WAITBATS:
                for b in range(2 if LV == 5 else 1):
                    bx = (rd("_bat_fx", b * 2) | (rd("_bat_fx", b * 2 + 1) << 8)) >> 4
                    if abs(bx + 8 - x) < 30: bats_near = True
            if bats_near: pass
            elif glow or rnd.random() > GLOW: p.button("a"); mode = "pick"; wait = 10
            else:
                occ = [48 + 24 * s for s in range(5) if rd("_slot_type", s) != 0xFF]
                if occ:
                    near = min(occ, key=lambda c: abs(c - x))
                    target = x + (2 if near > x else -2) + int(rnd.gauss(0, 1)); mode = "move"
    p.tick(); f += 1
wins = sum(r[0] for r in res); n = len(res)
avg_stars = (sum(r[3] for r in res if r[0]) / wins) if wins else 0
print(f"L{LV} sig={SIG} glow={GLOW} react={RMIN}-{RMAX}: rounds={n} wins={wins} ({(wins/n*100 if n else 0):.0f}%) avg_stars={avg_stars:.1f} "
      f"sneaks={sneaks} shoos={shoos} steals={steals} jiggles={jig} cracks={cracks} batknocks={knocks} {'STUCK' if stuck else ''}")
print("  ", "".join("W" if r[0] else "L" for r in res))
p.stop(save=False)
