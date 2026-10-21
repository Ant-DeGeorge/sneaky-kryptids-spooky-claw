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
def start_level(lv):
    # back to title via reset is hard; instead poke level at title and press A
    wr("_level", lv); press("a"); tick(60); shot(f"L{lv}_card"); tick(200); wait_idle(); shot(f"L{lv}_play")

tick(200); shot("title")
# ---- W1-1
start_level(0)
goto(48 + 24 * occ()[0]); press("a"); tick(110); shot("L0_grab"); wait_idle()
# ---- W2-1 tombstones: need a fresh enter via title.. we can't, so poke level & call new round via stars? Simplest:
# force win -> stars -> next level flows naturally. Poke pscore=4 and hit.
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
force_win()                       # -> L1 (W1-2) play directly
wait_idle(); shot("L1_play")
wr("_mis_pendi", 2); tick(5); goto(48 + 24 * occ()[0]); tick(60); shot("L1_scoot"); wait_idle()
force_win()                       # -> card W2 -> L2
shot("W2_card"); tick(200); wait_idle(); shot("L2_play_tombstone")
# grab the tombstone slot
st = [s for s in range(5) if rd("_slot_type", s) != 0xFF and rd("_slot_sto", s)]
if st:
    goto(48 + 24 * st[0]); press("a"); tick(105); shot("L2_crack"); tick(12); shot("L2_crack2"); wait_idle(); shot("L2_revealed")
force_win(); wait_idle(); shot("L3_play")
wr("_pscore", 1); wr("_sack_typ", 0, 0); wr("_mis_pendi", 1); wr("_sneak_del", 5); tick(70); shot("L3_sneak"); press("a"); tick(8); shot("L3_bonk"); wait_idle()
force_win()                       # -> card W3 -> L4 bats
shot("W3_card"); tick(200); wait_idle(); shot("L4_bats")
# drop when a bat is near the drop column to show a knock
goto(80);
for _ in range(600):
    bx = rd("_bat_fx") | (rd("_bat_fx", 1) << 8)
    if abs((bx >> 4) + 8 - 80) < 20: break
    tick(1)
press("a"); tick(30); shot("L4_bat_knock"); tick(60); shot("L4_after_knock"); wait_idle()
force_win(); wait_idle(); shot("L5_two_bats")
force_win()                       # -> card W4 -> L6 space
shot("W4_card"); tick(200); wait_idle(); shot("L6_space"); tick(30); shot("L6_space2")
goto(48 + 24 * occ()[0]); press("a"); tick(60); shot("L6_floaty_drop"); wait_idle()
force_win(); wait_idle(); shot("L7_space_redcap")
wr("_pscore", 1); wr("_sack_typ", 0, 0); wr("_mis_pendi", 1); wr("_sneak_del", 5); tick(50); shot("L7_jet_sneak"); tick(80); wait_idle()
# lose screen on L7
wr("_rscore", 4)
for attempt in range(6):
    wait_idle(); wr("_rscore", 4)
    gap = next(x for x in range(40, 140) if all(abs(x - (48 + 24 * s)) > 14 for s in occ()))
    goto(gap); press("a")
    for _ in range(700):
        tick(1)
        if rd("_state") == 3: break
    if rd("_state") == 3: break
tick(60); shot("L7_lose"); press("a"); wait_idle()
# final win -> stars -> player 1 -> player 2 -> YOU WON
force_win()
tick(40); shot("final_p1"); tick(100); press("a"); tick(40); shot("final_p2"); press("a"); tick(10); shot("final_won"); tick(150); shot("final_won2"); tick(150); shot("final_won3")
press("a"); tick(60); shot("title_after")
p.stop(save=False)
print("done", k[0])
