#include "loader.h"
void ld_bkg_data(uint8_t bank, uint8_t first, uint8_t n, const uint8_t *data) {
    uint8_t b = _current_bank; SWITCH_ROM(bank); set_bkg_data(first, n, data); SWITCH_ROM(b);
}
void ld_spr_data(uint8_t bank, uint8_t first, uint8_t n, const uint8_t *data) {
    uint8_t b = _current_bank; SWITCH_ROM(bank); set_sprite_data(first, n, data); SWITCH_ROM(b);
}
void ld_screen(uint8_t bank, const uint8_t *map, const uint8_t *attr) {
    uint8_t b = _current_bank; SWITCH_ROM(bank);
    set_bkg_tiles(0, 0, 20, 18, map);
    VBK_REG = VBK_ATTRIBUTES; set_bkg_tiles(0, 0, 20, 18, attr); VBK_REG = VBK_TILES;
    SWITCH_ROM(b);
}
void ld_submap(uint8_t bank, uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t *map, const uint8_t *attr) {
    uint8_t b = _current_bank; SWITCH_ROM(bank);
    set_bkg_submap(x, y, w, h, map, 20);
    set_bkg_submap_attributes(x, y, w, h, attr, 20);
    SWITCH_ROM(b);
}
