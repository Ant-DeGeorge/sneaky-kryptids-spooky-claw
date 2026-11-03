# Sneaky Kryptid's Spooky Claw - Game Boy Color (128 KB, MBC5)
GBDK_HOME ?= $(HOME)/tools/gbdk
LCC = $(GBDK_HOME)/bin/lcc

NAME   = spooky_claw3
TITLE  = SPOOKYCLAW
GAME   = src/main0.c src/loader.c src/main.c src/sound.c src/sound_data.c src/assets0.c src/assets1.c src/assets2.c
CARD   = src/studio_card/studio_card.c src/studio_card/studio_card_home.c src/studio_card/studio_card_gfx.c src/studio_card/studio_card_scx.c
OBJ    = $(patsubst src/%.c,build/obj/%.o,$(GAME) $(CARD))

# -Wm-yC CGB only; -yo8 eight 16 KB ROM banks (128 KB: game + studio card); -Wm-ya1 one RAM bank (8 KB save)
# -autobank lets the studio card place itself in free banks (the game's own banks are fixed with #pragma bank)
CFLAGS = -Wa-l -Wl-m -Wl-j -Wm-yC -Wm-yn"$(TITLE)" -Wl-yo8 -Wm-yo8 -autobank -Isrc/studio_card

all: build/$(NAME).gbc build/$(NAME)_nosave.gbc

build/obj/studio_card/%.o: src/studio_card/%.c $(wildcard src/studio_card/*.h)
	mkdir -p $(dir $@)
	$(LCC) $(CFLAGS) -Wf--opt-code-speed -c -o $@ $<

build/obj/%.o: src/%.c src/assets.h src/sound.h
	mkdir -p $(dir $@)
	$(LCC) $(CFLAGS) -c -o $@ $<

src/assets1.c src/assets2.c src/assets0.c src/assets.h: tools/gen_assets.py tools/art.py
	python3 tools/gen_assets.py

src/sound_data.c: tools/gen_sound.py
	python3 tools/gen_sound.py

# MBC5 + RAM + BATTERY (0x1B): keeps progress on carts that have save memory
build/$(NAME).gbc: $(OBJ)
	mkdir -p build
	$(LCC) $(CFLAGS) -Wm-ya1 -Wm-yt0x1B -o $@ $(OBJ)

# plain MBC5 (0x19): same game, no save attempt
build/$(NAME)_nosave.gbc: $(OBJ)
	mkdir -p build
	$(LCC) $(CFLAGS) -Wm-yt0x19 -o $@ $(OBJ)

clean:
	rm -rf build/*.gbc build/*.map build/*.noi build/*.ihx src/*.o src/*.lst src/*.asm src/*.sym

.PHONY: all clean
