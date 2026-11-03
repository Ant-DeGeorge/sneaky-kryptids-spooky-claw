/*
 * studio_card.h — "Rejected By The Machines" studio card (X1 Static Awakening)
 * GBDK-2020 4.5.0, Game Boy Color only.
 *
 * Usage (from anywhere, it is BANKED):
 *     studio_card_play();      // ~5.5 s, A or START skips
 *
 * On return:
 *   - screen is black (all BG/OBJ palettes black), LCD on, LCDC/IE/CPU speed restored
 *   - SCX = SCY = 0, LCD STAT interrupt removed, all 40 sprites hidden
 *   - VRAM tiles, BG map (0x9800) and its CGB attribute map are left dirty:
 *     load your own tiles/map/attributes/palettes before showing anything
 *   - sound channels silenced, NR50/NR51/NR52 left on (0x77 / 0xFF / 0x80)
 * Call it before you start your music driver.
 */
#ifndef STUDIO_CARD_H
#define STUDIO_CARD_H
#include <gb/gb.h>
#include <stdint.h>

BANKREF_EXTERN(studio_card)
void studio_card_play(void) BANKED;

/* frame counter of the running card (0..329), handy for debugging/tests */
extern volatile uint16_t sc_frame;

#endif
