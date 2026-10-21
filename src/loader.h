#ifndef LOADER_H
#define LOADER_H
#include <gb/gb.h>
#include <stdint.h>
// ROM0 helpers: switch to the data bank, copy, switch back to whatever bank was running.
void ld_bkg_data(uint8_t bank, uint8_t first, uint8_t n, const uint8_t *data);
void ld_spr_data(uint8_t bank, uint8_t first, uint8_t n, const uint8_t *data);
void ld_screen(uint8_t bank, const uint8_t *map, const uint8_t *attr);
void ld_submap(uint8_t bank, uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t *map, const uint8_t *attr);
#endif
