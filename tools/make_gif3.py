import sys, re
from pyboy import PyBoy
rom, mapf, out = sys.argv[1], sys.argv[2], sys.argv[3]
syms = {m.group(2): int(m.group(1), 16) for m in re.finditer(r"([0-9A-F]{8})\s+(_\w+)", open(mapf).read())}
p = PyBoy(rom, window="null", cgb=True, sound_emulated=False); M = p.memory
rd = lambda n, o=0: M[syms[n[:9]] + o]
def wr(n, v, o=0): M[syms[n[:9]] + o] = v
fr = []; k = [0]
def run(n, rec=True):
    for i in range(n):
        p.tick(); k[0] += 1
        if rec and k[0] % 3 == 0: fr.append(p.screen.image.convert("RGB").resize((320, 288), 0))
def goto(x):
    for _ in range(400):
        cx = rd("_clawX")
        if abs(cx - x) <= 1: break
        b = "right" if cx < x else "left"; p.button_press(b); run(1); p.button_release(b)
    run(6)
def idle():
    for _ in range(2000):
        if rd("_state") == 2 and rd("_pstate") == 1: return
        run(1)
def occ(): return [s for s in range(5) if rd("_slot_type", s) != 0xFF]
def force_win():
    for a in range(6):
        idle(); wr("_pscore", 4)
        for i in range(4): wr("_sack_typ", i, i)
        goto(48 + 24 * occ()[0]); p.button("a")
        for _ in range(700):
            run(1)
            if rd("_state") == 4: break
        if rd("_state") == 4: break
    run(140); p.button("a"); run(30)
run(170, False); run(40)
wr("_level", 2); p.button("a"); run(120); idle()                      # W2 card + tombstone
st = [s for s in occ() if rd("_slot_sto", s)]
goto(48 + 24 * (st[0] if st else occ()[0])); p.button("a"); idle(); run(10)
force_win(); idle()                                                  # W2-2
wr("_level", 4); wr("_pscore", 4); 
for i in range(4): wr("_sack_typ", i, i)
force_win(); run(60); idle(); run(120)                               # -> W3 card + bats
goto(48 + 24 * occ()[0]); p.button("a"); idle()
wr("_level", 6); force_win(); run(60); idle(); run(90)              # -> W4 card + space
wr("_level", 7); force_win()                                         # -> player 1
run(200); p.button("a"); run(120); p.button("a"); run(260)           # player 2, YOU WON
fr[0].save(out, save_all=True, append_images=fr[1:], duration=50, loop=0, optimize=True)
print(len(fr), "frames")
