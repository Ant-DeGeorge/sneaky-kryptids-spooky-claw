#!/usr/bin/env python3
"""Preschooler-ish bot for balance + soak testing (needs the -Dstatic= symbol build).
args: rom map frames seed aim_sigma glow_aware(0-1) react_min react_max"""
import sys, re, random
from pyboy import PyBoy
rom, mapf = sys.argv[1], sys.argv[2]
FR = int(sys.argv[3]); rnd = random.Random(int(sys.argv[4]))
SIG = float(sys.argv[5]); GLOW = float(sys.argv[6]); RMIN, RMAX = int(sys.argv[7]), int(sys.argv[8])
syms = {m.group(2): int(m.group(1), 16) for m in re.finditer(r"([0-9A-F]{8})\s+(_\w+)", open(mapf).read())}
p = PyBoy(rom, window="null", cgb=True, sound_emulated=False)
M = p.memory
rd = lambda n, o=0: M[syms[n[:9]] + o]
claw_pal = lambda: rd("_claw_pal")
f = 0; held = None; wait = 0; mode = "pick"; target = 0; react = None
res = []; last_state = None; last_change = 0; prev = None; stuck = False
lv_rounds = {}
sneaks = shoos = steals = jig = gold = 0
prev_p = None
while f < FR:
    st, ps = rd("_state"), rd("_pstate")
    if (st, ps) != prev:
        last_change = f
        if st == 2 and (prev is None or prev[0] != 2):
            win = rd("_end_win"); lvl = rd("_level")
            # level already adjusted on entering end screen; record level that was played
            played = lvl - 1 if (win and rd("_leveled")) else (lvl + (1 if (not win and rd("_losses_i") == 0 and lvl < 4 and False) else 0))
            res.append((win, rd("_pscore"), rd("_rscore")))
        if st == 1 and ps == 12: sneaks += 1
        if st == 1 and ps == 13: shoos += 1
        if st == 1 and ps == 10: steals += 1
        if st == 1 and ps == 5: jig += 1
        prev = (st, ps)
    if f - last_change > 4000 and not (st == 1 and ps in (1, 12)):
        print("STUCK", st, ps, f); stuck = True; break
    if held: p.button_release(held); held = None
    if wait: wait -= 1
    elif st in (0, 2, 3):
        if rnd.random() < 0.02: p.button("a"); wait = 20
    elif st == 1 and ps == 12:      # sneak: react after a delay
        if react is None: react = rnd.randint(RMIN, RMAX)
        react -= 1
        if react <= 0: p.button("a"); react = None; wait = 10
    elif st == 1 and ps == 1:
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
            glow = claw_pal() in (1, 3)
            if glow or rnd.random() > GLOW:
                p.button("a"); mode = "pick"; wait = 10
            else:
                # kid notices it's not glowing: nudge a bit toward the nearest prize
                occ = [48 + 24 * s for s in range(5) if rd("_slot_type", s) != 0xFF]
                if occ:
                    near = min(occ, key=lambda c: abs(c - x))
                    target = x + (2 if near > x else -2) + int(rnd.gauss(0, 1)); mode = "move"
    p.tick(); f += 1
wins = sum(1 for r in res if r[0]); n = len(res)
print(f"sig={SIG} glow={GLOW} react={RMIN}-{RMAX}: rounds={n} wins={wins} ({(wins/n*100 if n else 0):.0f}%) "
      f"level_now={rd('_level')+1} stars={rd('_stars')} sneaks={sneaks} shoos={shoos} steals={steals} jiggles={jig} "
      f"min/round={(f/60/60/max(n,1)):.1f} {'STUCK' if stuck else ''}")
print("results:", "".join("W" if r[0] else "L" for r in res))
p.stop(save=False)
