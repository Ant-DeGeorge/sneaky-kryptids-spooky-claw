#!/usr/bin/env python3
"""Records a short gameplay GIF for preview."""
import sys
from pyboy import PyBoy
p = PyBoy(sys.argv[1], window="null", cgb=True, sound_emulated=False)
frames = []
def run(n, rec=True):
    for i in range(n):
        p.tick()
        if rec and i % 3 == 0: frames.append(p.screen.image.convert("RGB").resize((320, 288), 0))
run(170, False); run(60)
p.button("a"); run(50)
p.button_press("right"); run(56); p.button_release("right"); run(20)
p.button("a"); run(330)
run(150)
p.button("a"); run(90)
p.button_press("right"); run(100); p.button_release("right"); run(10)
p.button("a"); run(380)
run(120)
frames[0].save(sys.argv[2], save_all=True, append_images=frames[1:], duration=50, loop=0, optimize=True)
print(len(frames), "frames")
