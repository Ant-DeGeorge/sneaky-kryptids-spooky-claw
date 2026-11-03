// ROM0 entry: studio card first, then the game (bank 1, see main.c).
#include <gb/gb.h>
#include "studio_card.h"
void game_main(void) BANKED;
void main(void) {
#ifndef NO_STUDIO_CARD     /* test builds can skip the card (PyBoy cannot time its raster) */
    if (_cpu == CGB_TYPE) {
        studio_card_play();   // Rejected By The Machines card, ~5.5 s, A/START skips
    }
#endif
    game_main();
}
