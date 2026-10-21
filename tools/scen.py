#!/usr/bin/env python3
"""Scenario screenshots: forces each mischief path through WRAM pokes (needs the -Dstatic= symbol build)."""
import sys, os, re
from pyboy import PyBoy
rom, mapf, out = sys.argv[1], sys.argv[2], sys.argv[3]
os.makedirs(out, exist_ok=True)
syms = {m.group(2): int(m.group(1), 16) for m in re.finditer(r"([0-9A-F]{8})\s+(_\w+)", open(mapf).read())}
A = lambda n: syms[n[:9]]
p = PyBoy(rom, window="null", cgb=True, sound_emulated=False)
M = p.memory
rd = lambda n, o=0: M[A(n) + o]
def wr(n, v, o=0): M[A(n) + o] = v
k = [0]
def tick(n=1):
    for _ in range(n): p.tick()
def shot(name):
    p.screen.image.convert("RGB").resize((320, 288), 0).save(f"{out}/{k[0]:02d}_{name}.png"); k[0] += 1
def press(b): p.button_press(b); tick(3); p.button_release(b); tick(1)
def goto(x):
    while rd("_clawX") < x: p.button_press("right"); tick(1)
    p.button_release("right")
    while rd("_clawX") > x: p.button_press("left"); tick(1)
    p.button_release("left"); tick(2)
def wait_idle(maxf=1500):
    for _ in range(maxf):
        if rd("_state") == 1 and rd("_pstate") == 1: return True
        tick(1)
    return False

tick(150); press("a"); wait_idle()
# 1 hit to have something in the sack
goto(48); press("a"); wait_idle()
shot("one_point")
# MISS -> steal
goto(60); press("a"); tick(150); shot("miss_empty"); tick(25); shot("steal_run"); tick(20); shot("steal_grab"); tick(40); shot("steal_leave")
wait_idle(); shot("pip_filled")
# SNEAK
wr("_mis_pendi", 1); wr("_sneak_del", 5); tick(20); shot("sneak_start"); tick(45); shot("sneak_mid")
press("a"); tick(6); shot("shoo_bonk"); tick(30); shot("shoo_tumble"); wait_idle()
# SNEAK success (no shoo)
wr("_mis_pendi", 1); wr("_sneak_del", 5); tick(170); shot("sneak_took"); tick(30); shot("sneak_leaving"); wait_idle(); shot("sack_after_take")
# SCOOT
wr("_mis_pendi", 2); tick(5)
for s in range(5):
    t = rd("_slot_type", s)
    if t != 0xFF: goto(48 + 24 * s); break
tick(40); shot("scoot_arrive"); tick(25); shot("scoot_lift"); tick(30); shot("scoot_carry"); wait_idle(); shot("scoot_done")
# JIGGLE (force: level 4 jiggle chance=5/16; poke level and loop until it triggers)
wr("_level", 4); wr("_jiggle_la", 0)
for attempt in range(12):
    for s in range(5):
        if rd("_slot_type", s) != 0xFF: tgt = 48 + 24 * s; break
    goto(tgt); press("a")
    for f in range(130):
        tick(1)
        if rd("_jiggle") == 1 and rd("_pstate") == 4: break
    if rd("_jiggle") == 1:
        tick(24); shot("jiggle_jump"); tick(10); shot("jiggle_stomp"); tick(10); shot("jiggle_fall"); wait_idle(); shot("jiggle_after")
        break
    wait_idle(); wr("_jiggle_la", 0)
wr("_level", 0)
# GOLDEN steal-back
wait_idle(); wr("_golden", 1); tick(3)
for s in range(5):
    if rd("_slot_type", s) != 0xFF: tgt = 48 + 24 * s; break
goto(tgt); shot("golden_ready"); press("a"); tick(250); shot("golden_fly1"); tick(30); shot("golden_steal_back"); wait_idle(); shot("golden_done")
print("pscore", rd("_pscore"), "rscore", rd("_rscore"))
# play out a LOSS: poke redcap to 4 and miss
wr("_rscore", 4); goto(60); press("a"); tick(400); shot("lose_screen"); tick(60); shot("lose_screen2")
press("a"); wait_idle()
# WIN: poke player to 4 and hit
wr("_pscore", 4)
for i in range(4): wr("_sack_type", i, i)
for s in range(5):
    if rd("_slot_type", s) != 0xFF: tgt = 48 + 24 * s; break
goto(tgt); press("a"); tick(420); shot("win_screen"); tick(40); shot("win_screen2")
press("a"); wait_idle(); shot("level2_start")
p.stop(save=False)
