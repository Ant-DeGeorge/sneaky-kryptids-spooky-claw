// ROM0 entry: the whole game lives in bank 1 (see main.c).
#include <gb/gb.h>
void game_main(void) BANKED;
void main(void) { game_main(); }
