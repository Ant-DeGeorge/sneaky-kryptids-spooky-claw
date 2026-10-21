import sys, os
from pyboy import PyBoy
rom, out = sys.argv[1], sys.argv[2]
os.makedirs(out, exist_ok=True)
p = PyBoy(rom, window="null", cgb=True, sound_emulated=False)
n = [0]
def tick(k=1):
    for _ in range(k): p.tick()
def shot(name):
    p.screen.image.convert("RGB").resize((320, 288), 0).save(f"{out}/{n[0]:02d}_{name}.png"); n[0] += 1
def press(b): p.button_press(b); tick(3); p.button_release(b); tick(1)
def hold(b, f): p.button_press(b); tick(f); p.button_release(b); tick(1)
tick(180); shot("title")
press("a"); tick(20); shot("ready"); tick(60); shot("go")
hold("right", 28); tick(4); shot("aim_glow")      # claw 20 -> 48 = slot 0 centre
press("a"); tick(100); shot("grab")
tick(90); shot("carry"); tick(60); shot("fly"); tick(30); shot("scored"); tick(60); shot("refill")
# deliberate miss: park between slots
hold("right", 12); tick(4); shot("miss_aim")
press("a"); tick(170); shot("miss_rise"); tick(30); shot("steal1"); tick(30); shot("steal2"); tick(40); shot("steal3"); tick(80); shot("after_steal")
p.stop(save=False)
