"""Preview GIF: hit, miss -> Redcap steals, Redcap sneak -> shoo (uses symbol build to find gaps)."""
import sys, re
from pyboy import PyBoy
rom, mapf, out = sys.argv[1], sys.argv[2], sys.argv[3]
syms = {m.group(2): int(m.group(1), 16) for m in re.finditer(r"([0-9A-F]{8})\s+(_\w+)", open(mapf).read())}
p = PyBoy(rom, window="null", cgb=True, sound_emulated=False); M = p.memory
rd = lambda n, o=0: M[syms[n[:9]] + o]
def wr(n, v): M[syms[n[:9]]] = v
fr = []; k = [0]
def run(n, rec=True):
    for i in range(n):
        p.tick(); k[0] += 1
        if rec and k[0] % 3 == 0: fr.append(p.screen.image.convert("RGB").resize((320, 288), 0))
def goto(x):
    while rd("_clawX") < x: p.button_press("right"); run(1)
    p.button_release("right")
    while rd("_clawX") > x: p.button_press("left"); run(1)
    p.button_release("left"); run(8)
def idle():
    while not (rd("_state") == 1 and rd("_pstate") == 1): run(1)
def occ(): return [48 + 24 * s for s in range(5) if rd("_slot_type", s) != 0xFF]
run(160, False); run(50); p.button("a"); idle(); run(10)
goto(occ()[0]); p.button("a"); idle(); run(10)                        # HIT
xs = occ(); gap = next(x for x in range(40, 140) if all(abs(x - c) > 12 for c in xs))
goto(gap); p.button("a"); idle(); run(10)                             # MISS -> steal
wr("_mis_pendi", 1); wr("_sneak_del", 10); run(80)                     # SNEAK...
p.button("a"); idle(); run(20)                                        # ...SHOO!
fr[0].save(out, save_all=True, append_images=fr[1:], duration=50, loop=0, optimize=True)
print(len(fr), "frames")
