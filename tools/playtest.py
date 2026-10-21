#!/usr/bin/env python3
"""Headless playtest: drives the ROM through title -> play -> grab -> reveal and saves screenshots."""
import sys, os
from pyboy import PyBoy

rom = sys.argv[1]
out = sys.argv[2] if len(sys.argv) > 2 else "build/shots"
os.makedirs(out, exist_ok=True)
p = PyBoy(rom, window="null", cgb=True, sound_emulated=False)
n = 0
def tick(k=1):
    for _ in range(k): p.tick()
def shot(name):
    global n
    p.screen.image.convert("RGB").resize((320, 288), 0).save(f"{out}/{n:02d}_{name}.png"); n += 1
def press(b, frames=2):
    p.button_press(b); tick(frames); p.button_release(b); tick(1)
def hold(b, frames):
    p.button_press(b); tick(frames); p.button_release(b); tick(1)

tick(120); shot("title")
press("a"); tick(30); shot("play_start")
press("a"); tick(20); shot("nope_over_chute")
hold("right", 24); tick(5); shot("moved")          # claw centre ~44 -> lower slot 0 (40)
press("a"); tick(45); shot("dropping")
tick(40); shot("grabbed")
tick(70); shot("rising_or_carry")
tick(120); shot("release")
tick(60); shot("reveal_early")
tick(80); shot("reveal")
press("a"); tick(40); shot("back_to_play")
tick(40); shot("refilled")
# miss: move to far right edge column beyond slots?  go over an empty floor area after removing
hold("right", 200); tick(5); shot("far_right")
# keep playing many rounds to reach the party screen
for rnd in range(40):
    if rnd % 2: hold("left", 40)
    else: hold("right", 40)
    press("a"); tick(420)
    press("a"); tick(80)
    if rnd in (3, 10):
        shot(f"round{rnd}")
shot("late")
tick(200); shot("late2")
p.stop(save=False)
