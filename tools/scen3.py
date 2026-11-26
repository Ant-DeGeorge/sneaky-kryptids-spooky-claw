#!/usr/bin/env python3
"""Forced-scenario screenshots for every level, obstacle and screen (symbol build)."""
import sys, os, re
from pyboy import PyBoy
rom, mapf, out = sys.argv[1], sys.argv[2], sys.argv[3]
os.makedirs(out, exist_ok=True)
syms = {m.group(2): int(m.group(1), 16) for m in re.finditer(r"([0-9A-F]{8})\s+(_\w+)", open(mapf).read())}
A = lambda n: syms[n[:9]]
p = PyBoy(rom, window="null", cgb=True, sound_emulated=False); M = p.memory
rd = lambda n, o=0: M[A(n) + o]
def wr(n, v, o=0): M[A(n) + o] = v
k = [0]
def tick(n=1):
    for _ in range(n): p.tick()
def shot(name):
    p.screen.image.convert("RGB").resize((320, 288), 0).save(f"{out}/{k[0]:02d}_{name}.png"); k[0] += 1
def press(b): p.button_press(b); tick(3); p.button_release(b); tick(1)
def goto(x):
    for _ in range(400):
        cx = rd("_clawX")
        if abs(cx - x) <= 1: break
        p.button_press("right" if cx < x else "left"); tick(1); p.button_release("right" if cx < x else "left")
    tick(2)
def wait_idle(maxf=1500):
    for _ in range(maxf):
        if rd("_state") == 2 and rd("_pstate") == 1: return True
        tick(1)
    return False
def occ(): return [s for s in range(5) if rd("_slot_type", s) != 0xFF]

def force_win():
    for attempt in range(8):
        wait_idle()
        wr("_pscore", 4)
        for i in range(4): wr("_sack_typ", i, i)
        goto(48 + 24 * occ()[0]); press("a")
        for _ in range(700):
            tick(1)
            if rd("_state") == 4: break
        if rd("_state") == 4: break
    tick(130); shot(f"stars_after_L{rd('_level')}"); press("a"); tick(80)
def turkey_knock(tag):
    goto(80)
    for _ in range(600):
        bx = rd("_bat_fx") | (rd("_bat_fx", 1) << 8)
        if abs((bx >> 4) + 8 - 80) < 20: break
        tick(1)
    press("a"); tick(30); shot(tag); wait_idle()

tick(200); shot("title")
# ---- W1-1 HARVEST: card (stalk + turkey + horn), corn stalk crunch, scarecrow plant, turkey knock
wr("_level", 0); press("a"); tick(60); shot("W1_harvest_card"); tick(200); wait_idle(); shot("L0_harvest")
st = [s for s in range(5) if rd("_slot_type", s) != 0xFF and rd("_slot_sto", s)]
if st:
    goto(48 + 24 * st[0]); press("a"); tick(105); shot("L0_crunch"); wait_idle(); shot("L0_after_crunch")
wr("_mis_pendi", 2); tick(5); goto(48 + 24 * occ()[0]); tick(45); shot("L0_scarecrow_pick"); tick(40); shot("L0_scarecrow_plant"); wait_idle()
turkey_knock("L0_turkey_knock")
force_win(); wait_idle(); shot("L1_harvest")
# ---- W1-2: plant far away + grow a stalk, scarecrow sneak + bonk
wr("_mis_pendi", 2); tick(5); goto(48 + 24 * occ()[0]); tick(80); shot("L1_plant_stalk"); tick(20); shot("L1_plant_stalk2"); wait_idle()
wr("_pscore", 1); wr("_sack_typ", 0, 0); wr("_mis_pendi", 1); wr("_sneak_del", 5); tick(70); shot("L1_scarecrow_sneak"); press("a"); tick(8); shot("L1_bonk"); wait_idle()
# lose screen in Harvest (warm rays, food loot)
for attempt in range(6):
    wait_idle(); wr("_rscore", 4)
    gap = next(x for x in range(40, 140) if all(abs(x - (48 + 24 * s)) > 14 for s in occ()))
    goto(gap); press("a")
    for _ in range(700):
        tick(1)
        if rd("_state") == 3: break
    if rd("_state") == 3: break
tick(60); shot("L1_lose"); press("a"); wait_idle()
force_win()                       # -> card W2 (pumpkin patch) -> L2
shot("W2_card"); tick(200); wait_idle(); shot("L2_play")
goto(48 + 24 * occ()[0]); press("a"); tick(110); shot("L2_grab"); wait_idle()
force_win(); wait_idle(); shot("L3_play")
wr("_mis_pendi", 2); tick(5); goto(48 + 24 * occ()[0]); tick(60); shot("L3_scoot"); wait_idle()
force_win()                       # -> card W3 (graveyard) -> L4
shot("W3_card"); tick(200); wait_idle(); shot("L4_play_tombstone")
st = [s for s in range(5) if rd("_slot_type", s) != 0xFF and rd("_slot_sto", s)]
if st:
    goto(48 + 24 * st[0]); press("a"); tick(105); shot("L4_crack"); wait_idle(); shot("L4_revealed")
force_win(); wait_idle(); shot("L5_play")
wr("_pscore", 1); wr("_sack_typ", 0, 0); wr("_mis_pendi", 1); wr("_sneak_del", 5); tick(70); shot("L5_sneak"); press("a"); tick(8); shot("L5_bonk"); wait_idle()
force_win()                       # -> card W4 (forest) -> L6 bats
shot("W4_card"); tick(200); wait_idle(); shot("L6_bats")
turkey_knock("L6_bat_knock")
force_win(); wait_idle(); shot("L7_two_bats")
force_win()                       # -> card W5 (space) -> L8
shot("W5_card"); tick(200); wait_idle(); shot("L8_space"); tick(30)
goto(48 + 24 * occ()[0]); press("a"); tick(60); shot("L8_floaty_drop"); wait_idle()
force_win(); wait_idle(); shot("L9_space_redcap")
wr("_pscore", 1); wr("_sack_typ", 0, 0); wr("_mis_pendi", 1); wr("_sneak_del", 5); tick(50); shot("L9_jet_sneak"); tick(80); wait_idle()
# final win -> stars -> player 1 -> player 2 -> YOU WON
force_win()
tick(40); shot("final_p1"); tick(100); press("a"); tick(40); shot("final_p2"); press("a"); tick(10); shot("final_won"); tick(150); shot("final_won2")
press("a"); tick(60); shot("title_after")
p.stop(save=False)
print("done", k[0])
