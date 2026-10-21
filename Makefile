# Sneaky Kryptid's Spooky Claw - Game Boy Color (64 KB, MBC5)
GBDK_HOME ?= $(HOME)/tools/gbdk
LCC = $(GBDK_HOME)/bin/lcc

NAME   = spooky_claw3
TITLE  = SPOOKYCLAW
SRC    = src/main0.c src/loader.c src/main.c src/sound.c src/sound_data.c src/assets0.c src/assets1.c src/assets2.c

# -Wm-yC CGB only; -Wm-yo4 four 16 KB ROM banks (64 KB); -Wm-ya1 one RAM bank (8 KB save)
CFLAGS = -Wa-l -Wl-m -Wl-j -Wm-yC -Wm-yn"$(TITLE)" -Wm-yo4

all: build/$(NAME).gbc build/$(NAME)_nosave.gbc

src/assets1.c src/assets2.c src/assets0.c src/assets.h: tools/gen_assets.py tools/art.py
	python3 tools/gen_assets.py

src/sound_data.c: tools/gen_sound.py
	python3 tools/gen_sound.py

# MBC5 + RAM + BATTERY (0x1B): keeps progress on carts that have save memory
build/$(NAME).gbc: $(SRC) src/assets.h src/sound.h
	mkdir -p build
	$(LCC) $(CFLAGS) -Wm-ya1 -Wm-yt0x1B -o $@ $(SRC)

# plain MBC5 (0x19): same game, no save attempt
build/$(NAME)_nosave.gbc: $(SRC) src/assets.h src/sound.h
	mkdir -p build
	$(LCC) $(CFLAGS) -Wm-yt0x19 -o $@ $(SRC)

clean:
	rm -rf build/*.gbc build/*.map build/*.noi build/*.ihx src/*.o src/*.lst src/*.asm src/*.sym

.PHONY: all clean
