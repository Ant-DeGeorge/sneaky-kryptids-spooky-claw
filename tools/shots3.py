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
tick(200); shot("title")
press("a"); tick(60); shot("card_w1"); tick(170); shot("ready")
tick(80); hold("right", 28); tick(4); shot("aim")
press("a"); tick(120); shot("grab"); tick(150); shot("fly"); tick(60); shot("after")
p.stop(save=False)
