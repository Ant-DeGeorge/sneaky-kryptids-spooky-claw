#!/usr/bin/env python3
"""Automated soak test. Reads game variables from WRAM (test build with exported symbols).
mode 'kid'  : random claw positions, random button mashing between turns
mode 'aim'  : aims at prizes the player doesn't have yet
"""
import sys, re, os, random
from pyboy import PyBoy

rom, mapf, mode = sys.argv[1], sys.argv[2], sys.argv[3]
frames_limit = int(sys.argv[4]) if len(sys.argv) > 4 else 60000
outdir = sys.argv[5] if len(sys.argv) > 5 else None
syms = {}
for m in re.finditer(r"([0-9A-F]{8})\s+(_\w+)", open(mapf).read()):
    syms[m.group(2)[:9]] = int(m.group(1), 16)
A = lambda n: syms[n[:9]]
p = PyBoy(rom, window="null", cgb=True, sound_emulated=False)
mem = p.memory
rd = lambda n, o=0: mem[A(n) + o]
rnd = random.Random(int(sys.argv[6]) if len(sys.argv) > 6 else 1)
SLOT_CX = [40, 72, 104, 136, 56, 88, 120]

f = 0; stats = dict(drops=0, reveals=0, parties=0, misses=0, slips=0, nopes=0)
prev_state = None; prev_pstate = None; stuck = 0; last_change = 0
shots = set()
def shot(name):
    if outdir and name not in shots:
        shots.add(name); os.makedirs(outdir, exist_ok=True)
        p.screen.image.convert("RGB").resize((320, 288), 0).save(f"{outdir}/{name}.png")

target = None; wait = 0; held = None; party_shot_at = -1
reveals_at_party = []
while f < frames_limit:
    st, ps = rd("_state"), rd("_pstate")
    if (st, ps) != (prev_state, prev_pstate):
        last_change = f
        if st == 2 and prev_state != 2: stats["reveals"] += 1
        if st == 3 and prev_state != 3: party_shot_at = f + 30
        if False:
            pass
        if st == 3 and prev_state != 3:
            stats["parties"] += 1; reveals_at_party.append(stats["reveals"])
        if st == 1 and ps == 4: stats["slips"] += 1; shot("slip")
        prev_state, prev_pstate = st, ps
    if st == 1 and rd("_boo_t") == 60: stats["misses"] += 1; shot("miss_ghost")
    if f - last_change > 3000 and not (st == 1 and ps == 0):
        print("STUCK", st, ps, f); break
    if held:
        p.button_release(held); held = None
    if wait: wait -= 1
    elif st == 0:
        p.button("a"); wait = 10
    elif st in (2, 3):
        if (st == 2 and rd("_scr_time") > 75) or (st == 3 and rd("_scr_time") > 125):
            if mode == 'kid' and rnd.random() < 0.5: wait = rnd.randint(0, 200)   # sometimes let it auto-advance
            else: p.button("a"); wait = 10
    elif st == 1 and ps == 0:
        x = rd("_clawX")
        if target is None:
            if mode == 'aim':
                col = [rd("_collected", i) for i in range(8)]
                types = [rd("_slot_typ", i) for i in range(7)]
                cand = [i for i in range(7) if types[i] != 0xFF and not col[types[i]]] or [i for i in range(7) if types[i] != 0xFF]
                # the upper row blocks access only physically; aim at the exact centre
                target = SLOT_CX[rnd.choice(cand)] if cand else 80
            else:
                target = rnd.randint(20, 144)
        if x < target: p.button_press("right"); held = "right"
        elif x > target: p.button_press("left"); held = "left"
        else:
            if x < 32: stats["nopes"] += 1
            p.button("a"); stats["drops"] += 1; target = None; wait = 5
    if f == party_shot_at: shot('party')
    if st == 2 and rd('_scr_time') == 60: shot('reveal')
    p.tick(); f += 1

print(mode, stats, "reveals at each party:", reveals_at_party, "frames:", f, f"(~{f/60/60:.1f} min)")
p.stop(save=False)
